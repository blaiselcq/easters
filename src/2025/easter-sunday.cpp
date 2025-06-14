#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <span>
#include <string>
#include <vector>

struct Coord {
  std::uint8_t row;
  std::uint8_t col;
  auto operator<=>(const Coord &) const noexcept = default;
};

bool orthogonal(Coord lhs, Coord rhs) {
  return ((lhs.col > rhs.col) ? lhs.col - rhs.col : rhs.col - lhs.col) +
         ((lhs.row > rhs.row) ? lhs.row - rhs.row : rhs.row - lhs.row) == 1;
}

struct Input {
  std::vector<std::uint8_t> count_in_col;
  std::vector<std::uint8_t> count_in_row;
  std::set<Coord>           granites;
  std::set<Coord>           nodules;
};

auto line_to_vec(std::span<char> line) -> std::vector<std::uint8_t> {
  std::vector<std::uint8_t> result;
  result.reserve(line.size());
  return result;
}

auto parse_input() -> Input {
  Input result{};

  std::string line;
  std::getline(std::cin, line);

  for (char c : line) {
    if (c != ' ')
      result.count_in_col.push_back(static_cast<std::uint8_t>(c - 0x30));
  }
  const auto grid_size = result.count_in_col.size();
  result.count_in_row.reserve(grid_size);

  std::uint8_t l = 0;
  while (std::getline(std::cin, line)) {
    result.count_in_row.push_back(line[0] - 0x30);
    for (auto c = 0uz; c < grid_size; c++) {
      auto t = line[c + 1];
      switch (t) {
      case 'G':
        result.granites.emplace(l, c);
        break;
      case 'O':
        result.nodules.emplace(l, c);
        break;
      }
    }
    l++;
  }

  return result;
}

struct Node {
  std::set<Coord>::const_iterator nodules_it;
  std::set<Coord>                 mines = {};
  int                             checksum = 0;
};


void print_mines(const Input& input, std::set<Coord> mines) {
  std::cout << ' ';
  for (auto c: input.count_in_col) std::cout << static_cast<int>(c);
  std::cout << '\n';

  const auto size = input.count_in_col.size();
  for (std::uint8_t i = 0; i < size; i++) {
    std::cout << static_cast<int>(input.count_in_row[i]);
    for (std::uint8_t j = 0; j < size; j++) {
      if      (input.granites.contains(Coord{i, j})) std::cout << 'G';
      else if (input.nodules.contains(Coord{i, j}))  std::cout << 'O';
      else if (mines.contains(Coord{i, j}))          std::cout << 'M';
      else                                           std::cout << '.';
    }
    std::cout << '\n';
  }
  std::cout << std::endl;
}

auto explore(std::vector<Node>& to_explore, std::vector<std::pair<int, Node>>& leaves,
             const Input& input,
             std::set<Coord>::const_iterator stops_at) {
    const auto grid_size = input.count_in_col.size();
    auto node = std::move(to_explore.back());
    to_explore.pop_back();

    if (node.nodules_it == stops_at) {
      leaves.emplace_back(node.checksum, std::move(node));
      return;
    }

    const auto& nodule = *node.nodules_it;


    const auto try_add_to_explore = [&](Coord coord) -> bool {
      if (input.granites.contains(coord)
          or input.nodules.contains(coord)
          or node.mines.contains(coord))
        return false; // Mine already present

      std::uint8_t count_r = 0;
      std::uint8_t count_c = 0;
      for (auto& mine: node.mines) {
        if (mine.col == coord.col) count_c++;
        if (mine.row == coord.row) count_r++;
      }

      if (count_c >= input.count_in_col[coord.col] or
          count_r >= input.count_in_row[coord.row])
        return false; // Already enough mines in col / row

      
      const auto checksum = node.checksum + ((static_cast<int>(coord.col) + 1) xor
                                             (static_cast<int>(coord.row) + 1));
      auto mines = node.mines;
      mines.emplace(coord);
      to_explore.emplace_back(std::next(node.nodules_it), std::move(mines), checksum);
      return true;
    };

    const auto is_w = nodule.col == 0;
    const auto is_e = nodule.col == grid_size - 1;
    const auto is_n = nodule.row == 0;
    const auto is_s = nodule.row == grid_size - 1;

    if (not is_w)
      try_add_to_explore(Coord{nodule.row,
                               static_cast<uint8_t>(nodule.col - 1u)});
    if (not is_n)
      try_add_to_explore(Coord{static_cast<uint8_t>(nodule.row - 1u),
                               nodule.col});
    if (not is_e)
      try_add_to_explore(Coord{nodule.row,
                               static_cast<uint8_t>(nodule.col + 1u)});
    if (not is_s)
      try_add_to_explore(Coord{static_cast<uint8_t>(nodule.row + 1u),
                               nodule.col});
}

int main(int, char** argv) {
  const auto puzzle_number = *argv[1] - 0x30;

  auto input = parse_input();

  if (puzzle_number == 2) {
    for (auto& c: input.count_in_col) c *= 2;
    for (auto& c: input.count_in_row) c *= 2;

    input.nodules.insert(input.granites.cbegin(), input.granites.cend());
    input.granites.clear();
  }

  auto to_explore = std::vector<Node>{{.nodules_it = input.nodules.cbegin()}};
  std::vector<std::pair<int, Node>> leaves;

  while (not to_explore.empty()) {
    explore(to_explore, leaves, input, input.nodules.cend());
  }

  const auto max_it = 
    std::max_element(leaves.begin(), leaves.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first ;});

  // print_mines(input, max_it->second.mines);

  std::cout << max_it->first;
}
