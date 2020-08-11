CruzID: jwang456@ucsc.edu

My partner was Horacio Castillo Del Rio

HORACIO'S REPO CONTAINS THE FINAL VERSION OF THIS PROJECT.

I WROTE:
-AGENT.c
-MESSAGE.c
-FIELDTEST.c
-NEGOTIATIONTEST.c
-MESSAGETEST.c

We did not do extra credit, we both have Monday finals tmrw and were too busy cramming :'(

Introduction:
	The BattleBoats system is a system that relies on the state machine in Agent.c. Messages of various types are generated and
	encoded, then decoded and sent to Agent.c, where the state machine decides what moves to make based on the nature of the
	message. The system connects two UNO32s so they can play a game together. On whichever UNO32 btn4 is pressed, that UNO acts as
	the challenger. As challenger, it generates a random hash and sends it to the other UNO, which generates a random number in
	return. The two numbers are or'ed together to decide which UNO goes first, and the accepting UNO has a chance to verify the
	other UNO is not cheating. Then, the UNO's alternate between attacking and defending until one wins.

What worked well:
	I think out lab works well, the partner aspect was more of a challenge than expected. There was a bit of a failure of
	communication as I had a hard time explaining the issues with my partners files, and what needed to change. I think it would
	have been more time saving for me to do the project alone, as I ended up finishing before my partner and spent more time
	debugging his code than mine. Had I done it by myself I might ahve had the time to do the extra credit. One testing strategy
	I found effective was testing my files with some preliminary tests before waiting for the autograder to confirm them.

What I learned (and what didn't work well):
	I learned that communication is very important for working as partners. I ended up debugging part of my partner's code in
	Field.c and failed to comunicate well which parts I changed, resulting in him not knowing whcih parts needed to be kept when
	reverting to an older version of the code. I ended up writing more files than my partner, which I think could've been avoided
	with better time management on both our parts. I should have helepd debug his code earlier and communicated well why I made the
	changes I needed to make.

What I liked about the lab and what I would change:
	I liked the lab in general, I would just make more clear what is extra credit and what is not; I was rather confused by the
	example video. Other than that, I would not change much. It was a good lab and I really enjoyed doing it. The class this year
	has been a lot of fun in general, and I have found it to be the CS class I liked the most so far in my time at UCSC. The TAs and
	tutors were very accomodating and generous with their time; thank you guys for a great quarter!

