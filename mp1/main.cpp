#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>

class Node {
private:
  std::array<std::array<int, 3>, 3> _grid;
  int _g = 0, _h = 0;
  Node *_parent = nullptr;

  void _calcH() {
    _h = 0;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (_grid[i][j] != 0) {
          int expectedY = _grid[i][j] / 3;
          int expectedX = _grid[i][j] % 3;
          _h += std::abs(expectedY - i) + std::abs(expectedX - j);
        }
      }
    }
  }

public:
  Node(std::array<std::array<int, 3>, 3> grid, Node *parent) {
    _grid = grid;
    _parent = parent;
    if (parent != nullptr) {
      _g = parent->g() + 1;
    }
    _calcH();
  };

  int h() const { return _h; }
  int g() const { return _g; }
  int f() const { return _h + _g; }
  Node *parent() const { return _parent; }

  std::tuple<int, int> find(int val) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (_grid[i][j] == val) {
          return {i, j};
        }
      }
    }
    return {-1, -1};
  }

  std::array<std::array<int, 3>, 3> moveGrid(int y, int x) {
    std::array<std::array<int, 3>, 3> grid = _grid;

    int i, j;
    std::tie(i, j) = find(0);

    int newY = i + y;
    int newX = j + x;

    if (i == -1 || newX < 0 || newX > 2 || newY < 0 || newY > 2) {
      grid[0][0] = -1;
      return grid;
    }

    int temp = grid[newY][newX];

    grid[newY][newX] = grid[i][j];
    grid[i][j] = temp;

    _calcH();

    return grid;
  }

  void print() {
    std::cout << "g: " << _g << std::endl;
    std::cout << "h: " << _h << std::endl;
    std::cout << "f: " << _g + _h << std::endl;
    for (auto i : _grid) {
      std::cout << "|";

      for (auto j : i) {
        std::cout << " " << j << " |";
      }
      std::cout << std::endl;
    }
  }

  bool operator==(Node &rhs) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (_grid[i][j] != rhs._grid[i][j]) {
          return false;
        }
      }
    }

    return true;
  }
};

struct CompareNodes {
  bool operator()(const Node *lhs, const Node *rhs) const {
    if (lhs->f() == rhs->f()) {
      return lhs->h() > rhs->h();
    } else {
      return lhs->f() > rhs->f();
    }
  }
};

class SearchablePriorityQueue
    : public std::priority_queue<Node *, std::vector<Node *>, CompareNodes> {
public:
  std::vector<Node *>::iterator begin() {
    return std::priority_queue<Node *, std::vector<Node *>, CompareNodes>::c
        .begin();
  }
  std::vector<Node *>::iterator end() {
    return std::priority_queue<Node *, std::vector<Node *>, CompareNodes>::c
        .end();
  }
};

class Puzzle {
private:
  SearchablePriorityQueue _open;
  std::vector<Node *> _close;
  std::array<std::tuple<int, int>, 4> _moveset = {
      std::tuple<int, int>{1, 0},
      std::tuple<int, int>{-1, 0},
      std::tuple<int, int>{0, 1},
      std::tuple<int, int>{0, -1},
  };

public:
  Puzzle(std::array<std::array<int, 3>, 3> start) {
    Node *startNode = new Node(start, nullptr);
    _open.push(startNode);
  };

  Node *solve() {
    Node *curr = nullptr;
    while (true) {
      curr = _open.top();
      _open.pop();

      if (curr->h() == 0) {
        return curr;
      }

      for (int i = 0; i < 4; ++i) {
        int x, y;
        std::tie(x, y) = _moveset[i];
        std::array<std::array<int, 3>, 3> newGrid = curr->moveGrid(x, y);

        if (newGrid[0][0] == -1) {
          continue;
        }

        Node *newNode = new Node(newGrid, curr);

        for (auto node : _open) {
          if (*node == *newNode && newNode->g() > node->g()) {
            delete newNode;
            newNode = nullptr;
            goto end;
            break;
          }
        }

        for (auto node : _close) {
          if (*node == *newNode && newNode->f() >= node->f()) {
            delete newNode;
            newNode = nullptr;
            break;
          }
        }

      end:
        if (newNode != nullptr) {
          _open.push(newNode);
        }
      }

      _close.push_back(curr);
    }

    return nullptr;
  }

  std::vector<Node *> solution() {
    Node *final = solve();
    Node *curr = final;
    std::vector<Node *> ret;
    while (curr != nullptr) {
      ret.push_back(curr);
      curr = curr->parent();
    }

    return ret;
  }

  SearchablePriorityQueue open() const { return _open; }
  std::vector<Node *> close() const { return _close; }
};

int main() {
  auto start = std::chrono::high_resolution_clock::now();
  std::array<std::array<int, 3>, 3> test = {
      std::array<int, 3>{7, 2, 4},
      std::array<int, 3>{5, 0, 6},
      std::array<int, 3>{8, 3, 1},
  };

  Puzzle solver(test);
  std::vector<Node *> solution;
  solution = solver.solution();

  auto stop = std::chrono::high_resolution_clock::now();

  std::cout << "Solution: " << std::endl;
  Node *curr = nullptr;
  int i = 1;
  int last = solution.size();
  while (!solution.empty()) {
    std::cout << i << ". ";
    if (i == 1) {
      std::cout << "Start state" << std::endl;
    } else if (i == last) {
      std::cout << "Goal state" << std::endl;
    } else {
      std::cout << "Next state" << std::endl;
    }
    ++i;
    curr = solution.back();
    solution.pop_back();
    curr->print();
  }

  auto duration =
      std::chrono::duration_cast<std::chrono::seconds>(stop - start);

  std::cout << "Duration: " << duration.count() << std::endl;
  std::cout << "Total Nodes Visited: " << solver.close().size() << std::endl;

  return 0;
}
