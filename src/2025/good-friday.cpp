#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
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

using Field = std::vector<std::string>;

std::pair<GeneticMap, Field> parse_input() {
  GeneticMap result{};

  std::vector<std::string> lines{};
  std::vector<std::string> field{};

  std::string line;
  bool in_field = false;
  while (std::getline(std::cin, line)) {
    if (line.empty() and not lines.empty()) {
      in_field = true; continue;
    }
    if(in_field)
      field.emplace_back(line);
    else
      lines.emplace_back(line);
    
  }

  auto plants = std::map<Plant, std::pair<int, int>>{};

  // First get all the plant positions on the map
  for (const auto& [l, line]: std::ranges::views::enumerate(lines)) {
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

  return std::make_pair(std::move(result), std::move(field));
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

std::optional<Cost> distance(const GeneticMap& input, Plant from, Plant to) {
  std::vector<std::tuple<
    Plant,                  // Current
    Cost,                   // Current Cost
    std::pair<Cost, Plant>, // Link
    std::vector<Plant>      // Visited
  >> explore_stack;

  if (from == to) return std::make_optional(0);

  // static constexpr auto line_size = 26;
  // static std::array<unsigned short, line_size * line_size> distances{};

  // if (distances[(from-0x30)*line_size + (to - 0x30)] != 0)
  //   return distances[(from-0x30)*line_size + (to - 0x30)];

  std::optional<Cost> best_cost = std::nullopt;

  const auto eqr = input.links.equal_range(from);
  for (auto it = eqr.first; it != eqr.second; it++) {
    explore_stack.emplace_back(
      it->first, 0, it->second, std::vector<Plant>{from});
  }

  while (not explore_stack.empty()) {
    auto [plant, cost, link, visited] = explore_stack.back();
    explore_stack.pop_back();

     cost += link.first;
     plant = link.second;

    if (plant == to) {
      best_cost.emplace(best_cost ? std::min(*best_cost, cost) : cost);
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

  // if (best_cost)
  //   distances[(from-0x30)*line_size + (to - 0x30)] = *best_cost;
  return best_cost;
}

Cost best_distance(const GeneticMap& input, const std::map<Plant, unsigned short>& categories,
                  const std::vector<Plant>& extant,
                  Plant to) {
  Cost best_distance = INT_MAX;
  const auto cat_to = categories.at(to);
  for (const auto e: extant) {
    if (categories.at(e) == cat_to)
      best_distance = std::min(best_distance, distance(input, e, to).value());
  }

  return best_distance;
}

std::string get_categories(const std::map<Plant, unsigned short>& categories,
                           std::string line) {
  for (auto& c: line) {
    c = categories.at(c);
  }
  return line;
}

Cost line_cost(const GeneticMap &input,
               const std::map<Plant, unsigned short> &categories,
               const std::vector<Plant> &extant, const std::string &line,
               const std::string expected) {
  auto expected_categories = get_categories(categories, expected);

  static constexpr auto placeholder = '\1';

  std::map<std::string, Cost> working_lines{{line, 0}};
  std::map<std::string, Cost> new_working_lines{};

  const auto insert = [&new_working_lines](std::string line, Cost cost) {
    const auto it = new_working_lines.find(line);
    if (it != new_working_lines.end())
      new_working_lines[line] = std::min(new_working_lines[line], cost);
    else
      new_working_lines[line] = cost;
  };

  // Remove extra plants if the line is too long
  for (auto i = expected.size(); i < line.size(); i++) {
    new_working_lines.clear();
    for (auto [wl, cost] : working_lines) {
      for (auto c = 0uz; c < wl.size(); c++) {
        auto nl = wl;
        nl.erase(nl.begin() + c);
        insert(nl, cost + 300);
      }
    }
    std::swap(new_working_lines, working_lines);
  }

  // Add placholders if the line is too long
  for (auto i = line.size(); i < expected.size(); i++) {
    new_working_lines.clear();
    for (auto [wl, cost] : working_lines) {
      for (auto c = 0uz; c <= wl.size(); c++) {
        auto nl = wl;
        nl.insert(nl.begin() + c, placeholder);
        insert(nl, cost);
      }
    }
    std::swap(new_working_lines, working_lines);
  }

  for (auto i = 0u; i < expected.size(); i++) {
    new_working_lines.clear();
    for (auto [wl, cost] : working_lines) {
    assert(wl.size() == expected.size());

      // Keep configuration as is
      insert(wl, cost);

      // Remove letter and add placeholder
      auto nl = wl;
      nl.erase(nl.begin() + i);
      for (auto j = 0u; j <= nl.size(); j++) {
        auto nnl = nl;
        nnl.insert(nnl.begin() + j, placeholder);
        insert(nnl, cost + 300);
      }
    }
    std::swap(new_working_lines, working_lines);
  }

  Cost min = INT_MAX;
  for (const auto &[line, base_cost] : working_lines) {
    auto i = 0;
    auto cost = base_cost;
    assert(line.size() == expected.size());
    for (auto c : line) {
      if (c != placeholder and categories.at(c) == expected_categories[i])
        cost += distance(input, c, expected[i]).value();
      else if (c == placeholder)
        cost += best_distance(input, categories, extant, expected[i]);
      else {
        cost = INT_MAX;
        break;
      }
      i++;
    }
    min = std::min(cost, min);
  }

  return min;
}

const auto expected = std::array{
  "XVXVXVXBX",
  "VXVXVXVXB",
  "XTXVXAXBX",
  "TXVXXXAXB",
  "XTXVWAXBX",
  "TXVXVXAXB",
  "XTXVXAXBX",
  "TXAXAXAXA",
  "XTXAXAXAX",
};

int main(int, char** argv) {
  const auto puzzle_number = *argv[1] - 0x30;

  const auto [input, field]      = parse_input();
  const auto categories          = categorize(input);
  const auto extants             = get_extants(input);

  if (puzzle_number == 1) {
    auto sum = 0;

    for (auto i = 0u; i < extants.size(); i++) {
      for (auto j = i+1; j < extants.size(); j++) {
        auto p_i = extants[i]; auto p_j = extants[j];
        if (categories.at(p_i) != categories.at(p_j)) continue;
        sum += distance(input, p_i, p_j).value();
      }
    }
    std::cout << sum;
  }

  // Puzzle 2 is currently not producing the correct result
  if (puzzle_number == 2) {
    auto sum = 0;

    for (auto i = 0u; i < expected.size(); i++) {
      auto lc = line_cost(input, categories, extants, field[i], expected[i]);
      // std::cout << field[i] << " "  << lc << "\n";
      sum += lc;
    }
    std::cout << sum;
  }

  return 0;
}
