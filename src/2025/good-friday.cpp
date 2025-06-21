#include <algorithm>
#include <climits>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

using Plant = char;
using Cost = int;

struct GeneticMap {
  std::vector<Plant>                           plants;
  std::multimap<Plant, std::pair<Cost, Plant>> links;
};

GeneticMap parse_input() {
  GeneticMap result{};

  std::vector<std::string> lines{};

  std::string line;
  while (std::getline(std::cin, line)) {
    lines.emplace_back(line);
  }

  auto plants = std::map<Plant, std::pair<int, int>>{};

  // First get all the plant positions on the map
  for (const auto& [l, line]: std::ranges::views::enumerate(lines)) {
    if( line == "" ) break;
    for (auto [c, maybe_plant]: std::ranges::views::enumerate(line)) {
      if (maybe_plant >= 'A' and maybe_plant <= 'z') plants[maybe_plant] = {l, c};
    }
  }

  const auto get_in_map = [&lines](int lin, int col) -> std::optional<char> {
    if (lin < 0 or static_cast<std::size_t>(lin) >= lines.size()) return std::nullopt;
    const auto& line = lines[lin];
    if (col < 0 or static_cast<std::size_t>(col) >= line.size()) return std::nullopt;
    return std::make_optional(line[col]);
  };

  const auto create_links = [get_in_map, &result](
                            Plant plant_from, std::pair<int, int> from, char dir) {
    const auto offset_l = dir == 'u' ? -1 : dir == 'd' ? 1 : 0;
    const auto offset_c = dir == 'l' ? -1 : dir == 'r' ? 1 : 0;

    char plant_to = 0x0;

    std::string cost_digits{};

    auto coord = from;
    while (plant_to == 0x0) {
      coord.first += offset_l; coord.second += offset_c;

      auto symbol = get_in_map(coord.first, coord.second);
      if (symbol >= 'A' and symbol <= 'z') {
        plant_to = *symbol;
        break;
      }

      if (symbol >= '0' and symbol <= '9') {
        cost_digits.push_back(*symbol);
      }
    }

    if (dir == 'u' or dir == 'l')
      std::reverse(cost_digits.begin(), cost_digits.end());
    const auto cost = std::stoul(cost_digits);

    result.links.emplace(plant_from, std::make_pair(cost, plant_to));
  };


  // Then get all the links
  for (const auto [plant, coords]: plants) {
    const auto [l, c] = coords;
    if (auto maybe_link = get_in_map(l-1, c); maybe_link == '|')
      create_links(plant, coords, 'u');
    if (auto maybe_link = get_in_map(l+1, c); maybe_link == '|')
      create_links(plant, coords, 'd');
    if (auto maybe_link = get_in_map(l, c-1); maybe_link == '-')
      create_links(plant, coords, 'l');
    if (auto maybe_link = get_in_map(l, c+1); maybe_link == '-')
      create_links(plant, coords, 'r');
  }

  // Finally, copy the plants
  result.plants = std::vector<Plant>{};
  for (auto c: plants) result.plants.push_back(c.first);

  return result;
}

std::map<Plant, unsigned short> categorize(const GeneticMap& input) {
  std::map<Plant, unsigned short> res; 
  unsigned short i = 0;
  for (auto p: input.plants) res[p] = i++;

  // Categorize the graph while doing multiple pass
  // Stop when two passes produce the same result.
  auto sum = INT_MAX-1;
  auto ssum = sum+1;
  while (sum < ssum) {
    ssum = sum;
    sum = 0;
    for (auto [p, branch]: input.links) {
      auto v = std::min(res[p], res[branch.second]);
      res[p] = v;
      res[branch.second] = v;
      sum += v;
    }
  }

  return res;
}

std::vector<Plant> get_extants(const GeneticMap& input) {
  std::vector<Plant> res;

  for (auto p: input.plants)
    if (input.links.count(p) < 2) res.push_back(p);
  
  return res;
}

Cost distance(const GeneticMap& input, Plant from, Plant to) {
  std::vector<std::tuple<
    Plant,                  // Current
    Cost,                   // Current Cost
    std::pair<Cost, Plant>, // Link
    std::vector<Plant>      // Visited
  >> explore_stack;

  auto best_cost = INT_MAX;

  // Plant is extant, there is exactly one link
  const auto first_link = *input.links.find(from);
  explore_stack.emplace_back(
    first_link.first, 0, first_link.second, std::vector<Plant>{from});

  while (not explore_stack.empty()) {
    auto [plant, cost, link, visited] = explore_stack.back();
    explore_stack.pop_back();

     cost += link.first;
     plant = link.second;

    if (plant == to) {
      best_cost = std::min(best_cost, cost);
      continue;
    }

    const auto eqr = input.links.equal_range(plant);
    for (auto it = eqr.first; it != eqr.second; it++) {
      if (it->second.second != link.first and
          std::ranges::count(visited, it->second.second) == 0) {
        auto new_visited = visited;
        new_visited.push_back(plant);
        explore_stack.emplace_back(plant, cost, it->second, std::move(new_visited));
      }
    }
  }

  return best_cost;
}

int main(int, char** argv) {
  const auto puzzle_number = *argv[1] - 0x30;

  const auto input      = parse_input();
  const auto categories = categorize(input);
  const auto extants    = get_extants(input);

  auto sum = 0;

  for (auto i = 0u; i < extants.size(); i++) {
    for (auto j = i+1; j < extants.size(); j++) {
      auto p_i = extants[i]; auto p_j = extants[j];
      if (categories.at(p_i) != categories.at(p_j)) continue;
      sum += distance(input, p_i, p_j);
    }
  }

  std::cout << sum;
}
