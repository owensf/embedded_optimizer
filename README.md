# What does this do?
The scope of this project changed throughout in response to hardware limitations, but the fundamental premise is this: what would be the best March Madness bracket you could make, assuming
1. You knew what everyone else picked
2. You assumed every game was a random event with known probabilities
3. You did not know who won each game

It's a harder challenge than it sounds, and it came about because I do a march madness pool with my friends every year. I noticed that there was a lot of literature on the best way to structure your bracket beforehand (see here: https://www.bracketvoodoo.com/#!/content/strategy_guide_preview), but no tools about how to optimize your bracket if the precise outcomes are known! If you play with the same people every year, how do you plan to optimize your bracket for next year when you don't even know the optimal strategy for past years?

# Why a microcontroller?
Because it's fun! This is a silly project I did on an STM32 microcontroller I had left over from college. Is a microcontroller the best way to do this? Absolutely not, but I had fun. Python would obviously be the "correct" way to do this

# What does this actually do?

Like I said, the scope changed a lot as I realized how long running all this computationally-intensive stuff became. Over time, the scope narrowed to this: I took all 13 of the final four picks made in my bracket pool in 2023. Then, I calculated which one had the highest probability of winning (assuming you take fivethirtyeight win probabilities at face value). Even this limited scope takes several hours to calculate. I have to create a lot of test scenarios, and each one needs a bunch of calulations, because the list of ties changes every time.

The meat of it is in f4s.h and find_best_f4.c

f4s.h contains the data, including the final four picks, probabilities for different scnearios, etc.
team_decoder.txt turns the numbers of teams into real time neams
find_best_f4.c contains a lot of unfinished functions. The most interesting one that's finished is find_best_f4, which calculates the probability of the bracket that has the highest probability of winning. 
simulate_bracket.py is a sanity check I did in python to make sure the math on my find_best_f4.c was working right
