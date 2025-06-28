#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

using Replacements = std::map<std::vector<std::string>, std::vector<std::string>>;

std::pair<Replacements, std::vector<std::string>> parse_input() {
  auto replacements = Replacements{};

  std::string line;
  std::array<std::vector<std::string>, 2> lines;
  int p = 0;
  while (std::getline(std::cin, line)) {
    if (line.empty()) {
      p++;
      if (p % 2 == 0) {
        replacements[lines[0]] = lines[1];
        lines[0].clear(); lines[1].clear();
      }
      continue;
    }
    lines[p % 2].emplace_back(std::move(line));
  }

  return std::make_pair(std::move(replacements), std::move(lines[0]));
}

std::optional<std::vector<std::string>> replacement(std::span<const std::string> input,
                                                    const std::vector<std::string>& from,
                                                    const std::vector<std::string>& to,
                                                    int puzzle_number) {
  const auto it = input.begin();

  if (puzzle_number == 1) {
    if (from == std::vector(it, std::next(it, from.size())))
      return std::make_optional(to);
    return std::nullopt;
  }
 
  std::array<char, 10> replacements{0};
  for (auto i = 0; i < from.size(); i++) {
    if (from[i].size() != (*(it + i)).size()) return std::nullopt;
    if (from[i][0] != (*(it + i))[0]) return std::nullopt;

    for (auto j = 1; j < from[i].size(); j++) {
      auto c_from = from[i][j];
      auto c_input = (*(it + i))[j];

      if (c_input % 2 != c_from % 2) return std::nullopt;
      if (replacements[c_from - 0x30] != 0) {
        if (replacements[c_from - 0x30] != c_input) return std::nullopt;
      }
      replacements[c_from - 0x30] = c_input;
    }
  }
  auto result = std::vector<std::string>{};
  for (const auto& to_str: to) {
    auto& line = result.emplace_back(to_str);
    for (auto it = line.begin() + 1; it != line.end(); it++)
      *it = replacements[*it - 0x30];
  }
  return std::make_optional(std::move(result));
} 

std::vector<std::string> replace_puzzle(const Replacements& replacements,
                                        std::vector<std::string> input,
                                        int puzzle_number) {
  auto it = input.begin();
  while (it != input.end()) {
    for (const auto& [from, to]: replacements) {
      if (std::distance(it, input.end()) < from.size()) continue;
      if (auto rep = replacement({it, input.end()}, from, to, puzzle_number);
          rep) {
        it = input.insert(it, rep->begin(), rep->end());
        input.erase(std::next(it, rep->size()),
                    std::next(it, from.size() + rep->size()));
        break;
      }
    }
    it++;
  }
  return input;
}

int main(int, char **argv) {
  const auto puzzle_number = *argv[1] - 0x30;

  auto [replacements, input] = parse_input();

  auto sum = 0;
  auto psum = 1;

  while (psum != sum) {
    psum = sum;
    sum = 0;
    input = replace_puzzle(replacements, std::move(input), puzzle_number);
    for (const auto& line: input) sum += std::stoi(line);
    if (puzzle_number == 1) break;
  }

  std::cout << sum;
}
