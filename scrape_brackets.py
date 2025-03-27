#https://fantasy.espncdn.com/tournament-challenge-bracket/2023/en/api/v7/group?groupID=5313306
#https://fantasy.espncdn.com/tournament-challenge-bracket/2023/en/api/matchups
import requests
import os
import json
brackets = {}
base = 'https://fantasy.espncdn.com/tournament-challenge-bracket/2023/en/api/v7/group?length=10000&groupID='
def scrape_brackets(group_num):
    response = requests.get(base + group_num)
    #print(response)
    data = response.json()
    #print(data)
    #print(data["g"]["e"])
    for player in data["g"]["e"]:
        brackets[player["n_e"]] = player["ps"]
    return brackets

#print(scrape_brackets('5313306'))