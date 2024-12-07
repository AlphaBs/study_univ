# Tree

- node
- root node
- terminal node (leaf node)
- edge
- sibling node: 같은 부모를 가지는 노드들
- ancestor node: 간선을 따라 루트 노드까지 이르는 경로에 있는 모든 노드들
- descendant node: 서브 트리에 있는 하위 레벨의 노드들

- subtree: 부모 노드와 연결된 간선을 끊었을 때 생성되는 트리  
*각 노드는 자식 노드의 개수만큼 서브트리를 가진다*

- node degree: 노드의 차수, 노드에 연결된 자식 노드의 수
- tree degree: 트리에 있는 노드의 차수 중에서 가장 큰 값

### height

- node height: 루트에서 노드에 이르는 간선의 수. 노드의 레벨
- tree height: 트리에 있는 노드의 높이 중에서 가장 큰 값. 최대 레벨

### level

루트 노드의 레벨이 0부터 시작할수도, 1부터 시작할수도 있음 

# Binary Tree

- 공백이거나 루트와 왼쪽 서브 트리, 오른쪽 서브 트리의 두 개의 분리된 이진 트리로 구성된 노드의 유한 집합
- 왼쪽 서브 트리와 오른쪽 서브 트리 구별
- 레벨 i에서 최대 노드 수: 2^(i-1) (i >= 1)  
- 깊이가 k인 이진 트리가 가질 수 있는 최대 수: 2^k - 1(k >= 1)

## 편향 이진 트리 (skewed tree)

- 높이가 k일 때 k개의 노드를 가지면서 모든 노드가 왼쪽이나 오른쪽 한 방향으로만 서브 트리를 가지고 있는 트리

## 완전 이진 트리 (complete binary tree)

- 깊이가 k이고 노드 수가 n인 이진 트리
- 각 노드들이 깊이가 k인 포화 이진 트리에서 1부터 n까지 번호를 붙인 노드와 1:1로 일치
- 마지막 레벨을 제외 한 모든 레벨이 완전히 채워져 있음
- 마지막 레벨은 꽉 차 있지 않아도 되지만 노드가 왼쪽에서 오른쪽 방향으로 채워져야 함
- n노드 완전 이진 트리의 높이: `ceil(log2(n+1))`

## 전 이진 트리 (full binary tree)

- 모든 노드가 0개 혹은 2개의 자식 노드를 갖는 트리

## 포화 이진 트리 (perfect binary tree)

- 모든 레벨에 노드가 포화상태로 차 있는 이진 트리
- full binary tree 의 조건을 만족
- 모든 terminal node들의 level이 같음
- 깊이가 k일 때, 노드의 갯수가 정확히 `2^k-1` (k >= 0)

# Array Binary Tree

- 1차원 배열에 노드를 저장
- 높이가 h인 완전 이진 트리의 **노드 번호 == 인덱스**
- 인덱스 0번은 비워둠, 인덱스 1번부터 루트 노드 저장
- parent: `ceil(i/2)`
- left: `2i` (if `2i <= n`)
- right: `2i+1` (if `2i+1 <= n`)

# Linked Binary Tree

- 노드가 다른 노드의 포인터와 데이터를 가지고 있음
- (left, data, right) (+parent)

# 순회

V 의 위치에 따라 전중후위 결정

- 전위: VLR
- 후위: LRV
- 중위: LVR

