# Graph

G = (V, E), V는 정점, E는 간선

- 자기 간선 또는 자기 루프 없음
- 동일 간선의 중복 없음 (다중 그래프가 아닐 때)

## 무방향 그래프

- 간선의 방향이 없음
- 간선 표현: (Vi, Vj)

예시: V(G1) = {A,B,C}, E(G1) = {(A,B),(B,C)}

## 방향 그래프

- 간선 표현: Vi->Vj, <Vi, Vj>, <tail, head>

## 완전 그래프

연결 가능한 모든 간선이 연결된 상태

- 정점 n개 무방향 그래프 최대 간선 수 : n(n-1) / 2
- 정점 n개 방향 그래프 최대 간선 수 : n(n-1)

## 부분 그래프

일부 정점, 일부 간선으로 만든 그래프

## 가중 그래프

간선에 가중치 weight 존재

## 용어

- 인접 (adjacent): 간선 (Vi, Vj) 에서 두 정점 Vi와 Vj가 인접
- 부속 (incident): 간선 (Vi, Vj) 는 정점 Vi와 Vj에 부속