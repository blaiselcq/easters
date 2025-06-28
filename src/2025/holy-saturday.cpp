#include <iostream>
#include <string>
#include <vector>
#include <map>

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

std::vector<std::string> replace(const Replacements& replacements,
                                 std::vector<std::string> input) {
  auto it = input.begin();
  while (it != input.end()) {
    for (const auto& [from, to]: replacements) {
      if (std::distance(it, input.end()) < from.size()) continue;
      if (from == std::vector(it, std::next(it, from.size()))) {
        it = input.insert(it, to.begin(), to.end());
        input.erase(std::next(it, to.size()),
                    std::next(it, from.size() + to.size()));
        break;
      }
    }
    it++;
  }
  return input;
}

int main(int, char **argv) {
  auto [replacements, input] = parse_input();

  input = replace(replacements, std::move(input));

  auto sum = 0;
  for (const auto& line: input) sum += std::stoi(line);

  std::cout << sum;

  // const auto puzzle_number = *argv[1] - 0x30;
}
