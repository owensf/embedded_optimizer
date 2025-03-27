import random
from scrape_brackets import scrape_brackets
from bracket_format import team_decoder
inv_team_decoder = {v: k for k, v in team_decoder.items()}


def load_fivethirtyeight(filename):
    prob_dict = {}
    with open(filename, "r") as f:
        team_probabilities = [line.split() for line in f.readlines()]
        for team in team_probabilities:
            prob_dict[inv_team_decoder[team[6]]] = {}
            prob_dict[inv_team_decoder[team[6]]]["R64"] = float(team[0])
            prob_dict[inv_team_decoder[team[6]]]["R32"] = float(team[1])
            prob_dict[inv_team_decoder[team[6]]]["R16"] = float(team[2])
            prob_dict[inv_team_decoder[team[6]]]["R8"] = float(team[3])
            prob_dict[inv_team_decoder[team[6]]]["R4"] = float(team[4])
            prob_dict[inv_team_decoder[team[6]]]["R2"] = float(team[5])
    return prob_dict

# bart only
def load_probabilities(filename):
    prob_dict = {}
    with open(filename, "r") as f:
        team_probabilities = [line.split() for line in f.readlines()]
        for team in team_probabilities:
            prob_dict[inv_team_decoder[team[2]]] = {}
            prob_dict[inv_team_decoder[team[2]]]["R64"] = float(team[4]) /100
            prob_dict[inv_team_decoder[team[2]]]["R32"] = float(team[5]) /100
            prob_dict[inv_team_decoder[team[2]]]["R16"] = float(team[6]) /100
            prob_dict[inv_team_decoder[team[2]]]["R8"] = float(team[7]) /100
            prob_dict[inv_team_decoder[team[2]]]["R4"] = float(team[8]) /100
            prob_dict[inv_team_decoder[team[2]]]["R2"] = float(team[9]) /100
    return prob_dict


def format_brackets(group_number):
    formatted_brackets = {}
    unformatted_brackets = scrape_brackets(group_number)
    for name in unformatted_brackets:
        formatted_brackets[name] = [int(x) for x  in unformatted_brackets[name].split("|")]
    return formatted_brackets


def print_formatted_bracket(bracket):
    for team in bracket:
        print(team_decoder[team])



# generate random final four+ scenarios, only from teams that have > 10% chance at F4
def find_final_four_teams(prob_dict, pool_f4s):
    f4_teams = []
    best_final_four = []
    # find all teams with final four probability > 10%
    for i in range(1, 65):
        if prob_dict[i]["R8"] > 10:
            f4_teams.append(i)
    # generate final four scenarios. Each team must come from unique region
    # TODO: can this be computationally more efficient?
    # pick winner
    for team in f4_teams:
        new_f4 = [team]
        find_best_f4(pool_f4s, prob_dict)
    #return final_four_teams

# determine if teams are from different sides of the bracket
def different_sides(team1, team2):
    if 65 > team1 > 32 or team1 == 67 or team1 == 68:
        if team2 < 33 or team2 == 65 or team2 == 66:
            return True
    else:
        if 65 > team2 > 32 or team2 == 67 or team2 == 68:
            return True
    return False
# determine if teams are from the same side but different regions or not
def same_side_different_regions(team1, team2):
    # team1 south
    if team1 < 17 or team1 == 65:
        # team2 east
        if 16 < team2 < 33 or team2 == 66:
            return True
    # team1 east
    elif 16 < team1 < 33 or team1 == 66:
        # team2 south
        if team2 < 17 or team2 == 65:
            return True
    # team1 midwest
    elif 32 < team1 < 49 or team1 == 67:
        # team2 west
        if 65 > team2 > 48 or team2 == 68:
            return True
    # team1 west
    else:
        # team2 midwest
        if 32 < team2 < 49 or team2 == 67:
            return True
    return False

# format: champ, midwest/west, east/south, west, midwest, east, south
def find_best_f4(pool_f4s, prob_dict):
    f4_teams = set()
    num_scenarios = 0
    sum_probs = 0
    win_probs = []
    num_purdue_wins = 0
    purdue_prob = 0
    for i in range(len(pool_f4s)):
        win_probs.append(0)
    best_final_four = []

    # find all teams from final_four
    #print(pool_f4s[0])
    for f4 in pool_f4s:
        # only need to add f4[3/4/5/6] because winner is contained
        f4_teams.add(f4[3])
        f4_teams.add(f4[4])
        f4_teams.add(f4[5])
        f4_teams.add(f4[6])
    new_prob_dict = find_nonwinner_probs(prob_dict, f4_teams)
    print(new_prob_dict)
    # south non-winner
    f4_teams.add(65)
    # east non-winner
    f4_teams.add(66)
    # midwest non-winner
    f4_teams.add(67)
    # west non-winner
    f4_teams.add(68)
    #print("f4 teams", f4_teams)


    prob_win = {}
    for team in f4_teams:
        prob_win[team] = 0
    # enumerate all F4 scenarios in a loop. find winner in each scenario. add to winners's probability
    # find win_prob for each:

    # enumerate scenarios: format is [winner, runner-up, south/east loser, midwest/west loser]
    # pick champion
    for team_1 in f4_teams:
        # pick runner-up: has to be from opposite side of bracket
        for team_2 in f4_teams:
            # pick F4 losers (must be from opposite sides of brackets)
            if different_sides(team_1, team_2):
                    # F4 losers
                    # F4 loser to team1
                    for team_3 in f4_teams:
                        if same_side_different_regions(team_1, team_3):
                            # F4 loser to team2
                            for team_4 in f4_teams:
                                if same_side_different_regions(team_2, team_4):
                                    # figure out who wins scenario
                                    winners = id_winner(team_1, team_2, team_3, team_4, f4s)
                                    # figure out prob of scenario
                                    #print()
                                    #prob = new_prob_dict[team_1]["R2"] * (new_prob_dict[team_2]["R4"] - new_prob_dict[team_2]["R2"]) * (new_prob_dict[team_3]["R8"] - new_prob_dict[team_3]["R4"]) * (new_prob_dict[team_4]["R8"] - new_prob_dict[team_4]["R4"])
                                    prob = new_prob_dict[team_1]["R2"] * new_prob_dict[team_2]["R4"] * new_prob_dict[team_3]["R8"] * new_prob_dict[team_4]["R8"]

                                    sum_probs += prob
                                    num_scenarios += 1
                                    prob_win[team_1] += prob
                                    # add prob to winner
                                    '''print("Scenario: ", team_1, team_2, team_3, team_4)
                                    print("Prob:", prob)
                                    print("Winners", winners)
                                    print(new_prob_dict[team_1]["R2"])
                                    print((new_prob_dict[team_2]["R4"] - new_prob_dict[team_2]["R2"]))
                                    print((new_prob_dict[team_3]["R8"] - new_prob_dict[team_3]["R4"]))
                                    print((new_prob_dict[team_4]["R8"] - new_prob_dict[team_4]["R4"]))'''
                                    for winner in winners:
                                        win_probs[winner] += prob/len(winners)

    '''print(sum_probs)
    print(num_scenarios)
    print(prob_win)'''
    '''print("num purdue wins", num_purdue_wins)
    print("purdue + hourston", purdue_prob)'''
    # find highest ID
    print("num_scenarios", num_scenarios)
    winner = -1
    highest_win = 0
    for i in range(len(win_probs)):
        if win_probs[i] > highest_win:
            winner = i
            highest_win = win_probs[i]
    print(win_probs)
    return winner


# find winner in a specified scenario:
# team1: champ, team2: runner-up, team3: lost to team1, team4: lost to team2
def id_winner(team1, team2, team3, team4, f4s):
    max_points = -1
    winner_ids = []
    i = 0
    for f4 in f4s:
        points = 0
        if team1 == f4[0]:
            points += 320
        if team1 == f4[1] or team1 == f4[2]:
            points += 160
        if team1 == f4[3] or team1 == f4[4] or team1 == f4[5] or team1 == f4[6]:
            points += 80
        if team2 == f4[1] or team2 == f4[2]:
            points += 160
        if team2 == f4[3] or team2 == f4[4] or team2 == f4[5] or team2 == f4[6]:
            points += 80
        if team3 == f4[3] or team3 == f4[4] or team3 == f4[5] or team3 == f4[6]:
            points += 80
        if team4 == f4[3] or team4 == f4[4] or team4 == f4[5] or team4 == f4[6]:
            points += 80
        if points == max_points:
            winner_ids.append(i)
        if points > max_points:
            max_points = points
            winner_ids = [i]
        i += 1
    return winner_ids

# return all bracket ids that made a certain pick
# game_ids: 1: champ, 2: Midwest/west, 3: east/south, 4: west winner, 5: midwest winner, 6: east winner, 7: south winner
def return_f4_pickers(pool_f4s, team_id, game_id):
    bracket_ids = []
    for i in range(0, len(pool_f4s)):
        if pool_f4s[i][game_id-1] == team_id:
            bracket_ids.append(i)
    return bracket_ids

# modify prob_dict so that there's a sum of the non
def find_nonwinner_probs(prob_dict, f4_picks):
    new_prob_dict = prob_dict
    for i in range(4):
        new_prob_dict[65+i] = {}
        new_prob_dict[65+i]["R2"] = 0
        new_prob_dict[65+i]["R4"] = 0
        new_prob_dict[65+i]["R8"] = 0

    for team in prob_dict:
        if team < 17 and team not in f4_picks:
            new_prob_dict[65]["R2"] += prob_dict[team]["R2"]
            new_prob_dict[65]["R4"] += prob_dict[team]["R4"]
            new_prob_dict[65]["R8"] += prob_dict[team]["R8"]
        elif 16 < team < 33 and team not in f4_picks:
            new_prob_dict[66]["R2"] += prob_dict[team]["R2"]
            new_prob_dict[66]["R4"] += prob_dict[team]["R4"]
            new_prob_dict[66]["R8"] += prob_dict[team]["R8"]
        elif 32 < team < 49 and team not in f4_picks:
            new_prob_dict[67]["R2"] += prob_dict[team]["R2"]
            new_prob_dict[67]["R4"] += prob_dict[team]["R4"]
            new_prob_dict[67]["R8"] += prob_dict[team]["R8"]
        elif 48 < team < 65 and team not in f4_picks:
            new_prob_dict[68]["R2"] += prob_dict[team]["R2"]
            new_prob_dict[68]["R4"] += prob_dict[team]["R4"]
            new_prob_dict[68]["R8"] += prob_dict[team]["R8"]
    print(new_prob_dict)
    return new_prob_dict

# given brackets, return arrays of only the f4
# Order: champ, midwest/west, east/south, west winner, midwest winner, east winner, south winner
def return_f4s(bracket_pool):
    f4s = []
    for name in bracket_pool:
        bracket = bracket_pool[name]
        f4s.append([bracket[-1], bracket[-2], bracket[-3], bracket[-4], bracket[-5], bracket[-6], bracket[-7]])
    return f4s

def return_s16s(bracket_pool):
    s16s = []
    for name in bracket_pool:
        bracket=bracket_pool[name]
        s16=[]
        for i in range(1,32):
            s16.append(bracket[-i])
        s16s.append(s16)
    return s16s

#print(load_probabilities("barttorvik_probs.txt"))
#brackets = format_brackets('5724994')
brackets = format_brackets('5313306')
print("Brackets:", brackets)
print(return_s16s(brackets))

f4s = return_f4s(brackets)
#prob_dict = load_probabilities('barttorvik_probs.txt')
prob_dict = load_fivethirtyeight('fivethirtyeightwinprobs.txt')
ordered_dict = dict(sorted(prob_dict.items()))
print(ordered_dict)
for team in ordered_dict:
    print('{ ', end='')
    for item in ordered_dict[team]:
        print(ordered_dict[team][item], end='')
    print('}')
print("prob_dict", prob_dict)
print("Unique final four teams", find_final_four_teams(prob_dict, f4s))
print("Final fours:", f4s)
win_probs = find_best_f4(f4s, prob_dict)
print(win_probs)
for bracket in brackets:
    print(bracket)
#print("Winner in houston/uconn/princeton/kentucky f4", id_winner(33, 16, 55, 18, f4s))
#print(return_f4_pickers(f4s, 33, 1))

#for name in brackets:
#    print_formatted_bracket(brackets[name])