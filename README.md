# ranked

ranked는 redis의 sorted map와 같은 기능을 제공하며, periodic(daily, weekly, monthly, alltime etc..)한 데이터를 조회하는데에 최적화되어 있다.

## ranked의 필요성

기존 `redis`를 사용하는 경우 `expired zadd`나 `expired zincrby`를 구현하기 위해선 `redis`의 `setex`와 `psubscribe __keyevent::expired`를 이용하여 구현하거나, 서버의 자체 타이머(cron) 등을 이용하여 구현해야 했다.
이는 `stateless`하게 서버를 구현할 수 없는 요인이 되었으며, `expired`를 위한 부가적인 연산이 추가로 소요되었다.
`ranked`는 이러한 문제를 모두 `stateful` 노드에서 처리하게 함으로써 서버의 독립성을 높이는 프로젝트다.
`ranked`는 `redis`의 `zadd`, `zsub`, `zinc`, `zdec`, `zrange`, `zrevrange` `flushall`만을 `slim`하게 제공하며, 추가로 `ranked`의 핵심 함수인 `zincrbyp`를 제공한다.
`redis`가 제공하는 다른 연산들은 지원하지 않는다.

## ranked의 한계

`ranked`는 사용자가 실시간으로 `periodic`한 정보를 조회할 수 있도록 도와준다.

실시간 서비스가 필요하지 않을 경우, 가령 특정 시간에 한 번(하루에 한 번, 일주일에 한 번 등) 데이터를 재구성하는 경우엔 `ranked`말고 다른 방안을 강구해보는 것이 좋을 것이다.
그 이유는 `ranked`는 `redis`의 모든 기능을 제공하지 않으며, `incp`와 `decp` 데이터를 삽입하고 조회하는데에 특화되어 있기 때문이다.

`ranked`를 사용하지 않는 다른 방안으로는 `key`를 `timestamp`와 함께 저장하여 특정 `timestamp` 이상의 값들 만들 조회하는 방법을 사용할 수 있다.
다만 이 방법으로 실시간 처리를 구현한다면 매번 `O(n)`만큼의 시간이 소요될 것이고, 이는 실시간 랭킹 처리를 방해하는 큰 요인이 될 수 있음을 인지하여야 한다.

## ranked를 사용할 수 있는 경우

커뮤니티에서 특정 게시글의 조회수(또는 up vote 수 등)을 기준으로 실시간 게시글 랭킹 서비스를 구성한다고 하면 `ranked` 사용을 고려할 수 있다.
보통 당일에 올라온 글들이 실시간으로 높은 조회수를 가지게될 확률이 높지만, 다른 요인으로 인해 오래전 게시글이 많이 조회되는 경우도 있다.
당일에 올라온 글들만 실시간 게시글 랭킹을 허용한다면 다른 방법으로도 문제없겠지만, 모든 글들을 대상으로 실시간 랭킹 서비스를 구성한다면 `ranked` 사용을 고려해볼만 할 것이다.

## api

### zadd

```
zadd <table> [<value> <member>]+
```

`zadd`는 `table`에 있는 `member`를 `value`로 설정시킨다.
`value`는 정수만 가능하다.
`member` 값이 없는 경우 해당 값을 `value`로 설정시킨다.

### zincrby

```
zincrby <table> [<increment> <member>]+
```

`zincrby`는 `table`에 있는 `member`의 `value`를 `increment`만큼 증가시킨다.
`member` 값이 없는 경우 해당 값을 `0`으로 설정하고 `increment`만큼 증가시킨다.

### zincrbyp (expired zincrby)

```
zincrbyp <table> [<increment> <member>]+ <expire>
```

`zincrbyp`는 `table`에 있는 `member`의 `value`를 `increment`만큼 증가시키고, 
`expire`초가 지나면 `member`의 `value`를 `increment`만큼 감소시킨다.

### zrange

```
zrange <table> <offset> <count> [withscores]
```

`zrange`는 `<table>`에 있는 `key-value` 중 `value`가 작은 순으로 `key`들의 리스트를 가져오되, 
`offset`개 만큼 리스트의 앞 부분이 생략되고, `count`개 만큼의 리스트 크기만을 가져온다.
`withscores`는 `key-value`의 리스트를 가져온다.

### zrevrange

```
zrevrange <table> <offset> <count> [withscores]
```

`zrange`는 `<table>`에 있는 `key-value` 중 `value`가 큰 순으로 `key`들의 리스트를 가져오되, 
`offset`개 만큼 리스트의 앞 부분이 생략되고, `count`개 만큼의 리스트 크기만을 가져온다.
`withscores`는 `key-value`의 리스트를 가져온다.

### flushall

```
flushall
```

`flushall`은 메모리의 모든 `table`을 삭제한다.

## ranked의 구현

`ranked`는 `sorted map`과 `min heap`을 이용하여 구현한다.
`ranked`는 `incp` 또는 `decp` 요청을 해석하여 `remain`을 `timestamp`로 변환하고 `min heap`에 삽입한다.
`zrange`또는 `zrevrange`이 요청되면 `min heap`에 삽입된 항목들을 조회하고 `sorted map`를 재구성하고 `range`명령을 처리한다.

### range 연산의 구현

`sorted map`을 통하여 `range`연산을 구현하기 위해선, `sorted map`의 모든 원소를 `value`의 오름차순 또는 내림차순으로 정렬하고,
요구되는 `offset`과 `count`에 따라 결과를 리턴해야한다.
`sorted map`을 정렬하는데 `O(log n)`의 시간이 걸리므로, 한 번의 `range` 요청당 `log n`의 시간이 소요됨을 기대할 수 있다.
다만, `sorted map`을 정렬하기 위해선 전체를 복제해야하는데, 복제하는데 걸리는 시간이 병목이 될 것이다.
따라서 기존 `key`에 대한 정렬만 지원되는 `sorted map`을 확장하여 `key`와 `value` 모두의 정렬을 지원하는 `double sorted map`을 만든다.
`double sorted map`은 `key`에 대한 `btree`와 `value`에 대한 `btree`로 구현한다.
서로 상호 접근이 가능하게 포인터로 연결한다.

#### array 를 통한 구현

`double sorted map`을 기존 `std::map`을 통해서 구현하려면, `std::map`와 `sorted array`를 사용한다.
`sorted array`는 항상 정렬된 상태를 유지하는 `array`의 확장형이다.
`double sorted map`에 요소를 삽입한다면,

#### priority queue를 통한 구현

`heap`은 `up heap`과 `down heap`과정이 포함된다.
이 과정을 이용하면 `sorted map`과 `priority queue`를 이용하여
