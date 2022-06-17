// baekjoon 1260
// https://www.acmicpc.net/problem/1260

#include <iostream>
#include <cstdio>
#include <algorithm>
#include <stack>
#include <queue>

using namespace std;

int N, M, V;
bool map[1010][1010];

// 재귀호출, 함수호출스택사용
void dfs(int v)
{
    static bool visited[1010];

    cout << v << " ";
    visited[v] = true;
    for (int i = 1; i <= N; i++)
    {
        if (!visited[i] && map[v][i])
            dfs(i);
    }
}

// 반복, 스택 자료구조 직접 사용
void dfs_s(int v)
{
    static bool visited[1010];

    stack<int> stk;
    stk.push(v);

    while (!stk.empty())
    {
        int next = stk.top();
        stk.pop();
        if (!visited[next])
        {
            cout << next << " ";
            visited[next] = true;
        }

        for (int i = 1; i <= N; i++)
        {
            if (!visited[i] && map[v][i])
                stk.push(i);
        }
    }
}

void bfs(int v)
{
    static bool visited[1010];

    queue<int> q;
    visited[v] = true;
    q.push(v);
    while (!q.empty())
    {
        v = q.front();
        q.pop();
        cout << v << " ";

        for (int i = 1; i <= N; i++)
        {
            if (!visited[i] && map[v][i])
                q.push(i), visited[i] = true;
        }
    }
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);
    
    // input
    cin >> N >> M >> V;
    for (int i = 0; i < M; i++)
    {
        int a, b;
        cin >> a >> b;
        map[a][b] = map[b][a] = true;
    }

    dfs_s(V);
    cout << "\n";
    bfs(V);

    return 0;
}