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

  /* Calculate H using manhattan distance. With inspiration from:
   **https://stackoverflow.com/questions/39759721/calculating-the-manhattan-distance-in-the-eight-puzzle
   */
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

  /*
  ** Finds the x and y coordinates of the specified value in the cell
  */
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

  /*
  ** Creates a new grid based on the grid of the current node and the directions
  ** specified in the arguments.
  */
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

  /*
  ** Overloaded operator that compares if two nodes are equal. This means that
  ** they have exactly the same grid. Does not take f, g, or h into account
  */
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

  void print() {
    std::cout << "g: " << _g << std::endl
              << "h: " << _h << std::endl
              << "f: " << _g + _h << std::endl;
    for (auto i : _grid) {
      std::cout << "|";

      for (auto j : i) {
        if (j == 0) {
          std::cout << " _ |";
        } else {
          std::cout << " " << j << " |";
        }
      }
      std::cout << std::endl;
    }
  }
};

/*
** Struct to compare the different nodes for the priority queue
** https://stackoverflow.com/questions/16111337/declaring-a-priority-queue-in-c-with-a-custom-comparator
*/
struct CompareNodes {
  bool operator()(const Node *lhs, const Node *rhs) const {
    if (lhs->f() == rhs->f()) {
      return lhs->h() > rhs->h();
    } else {
      return lhs->f() > rhs->f();
    }
  }
};

/*
** Allow searching through priority queues.
** https://stackoverflow.com/questions/16749723/how-i-can-find-value-in-priority-queue
*/
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

  /*
  ** A* algorithm based on this pseudocode:
  ** https://mat.uab.cat/~alseda/MasterOpt/AStar-Algorithm.pdf
  ** https://www.geeksforgeeks.org/a-search-algorithm/
  */
  Node *solve() {
    Node *curr = nullptr;
    while (!_open.empty()) {
      curr = _open.top();
      _open.pop();

      // Found!
      if (curr->h() == 0) {
        return curr;
      }

      for (int i = 0; i < 4; ++i) {
        int x, y;
        std::tie(x, y) = _moveset[i];
        std::array<std::array<int, 3>, 3> newGrid = curr->moveGrid(x, y);

        // Skip if grid cannot be created
        if (newGrid[0][0] == -1) {
          continue;
        }

        // Create new possible node
        Node *newNode = new Node(newGrid, curr);

        // Check if found in open list
        for (auto node : _open) {
          if (*node == *newNode && newNode->g() > node->g()) {
            delete newNode;
            newNode = nullptr;
            break;
          }
        }

        // Check if found in closed list
        if (newNode != nullptr) {
          for (auto node : _close) {
            if (*node == *newNode && newNode->f() >= node->f()) {
              delete newNode;
              newNode = nullptr;
              break;
            }
          }
        }

        if (newNode != nullptr) {
          _open.push(newNode);
        }
      }

      // Add to closed list
      _close.push_back(curr);
    }

    return nullptr;
  }

  /*
  ** Retraces the steps from the solution back to the initial state. Stores it
  ** in a vector that acts as a stack.
  */
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

  // Create initial state
  std::array<std::array<int, 3>, 3> test = {
      std::array<int, 3>{7, 2, 4},
      std::array<int, 3>{5, 0, 6},
      std::array<int, 3>{8, 3, 1},
  };

  // Solve it
  Puzzle solver(test);
  std::vector<Node *> solution;
  solution = solver.solution();

  auto stop = std::chrono::high_resolution_clock::now();

  // Print solution
  std::cout << "Solution: " << std::endl;
  Node *curr = nullptr;
  int i = 1;
  int last = solution.size();
  while (!solution.empty()) {
    std::cout << std::endl << i << ". ";
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

  // Calculate statistics
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

  std::cout << std::endl
            << "Statistics: " << std::endl
            << "CPU Time: " << duration.count() << "ms" << std::endl
            << "Total Nodes Visited: " << solver.close().size() << std::endl;

  return 0;
}
