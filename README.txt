NOTE: When running the code, use the FieldCorrect object file. My partner was responsible for writing the Field.c file, and due to timing my version does not contain his finished code

Introduction:
	The BattleBoats system is a system that relies on the state machine in Agent.c. Messages of various types are generated and
	encoded, then decoded and sent to Agent.c, where the state machine decides what moves to make based on the nature of the
	message. The system connects two UNO32s so they can play a game together. On whichever UNO32 btn4 is pressed, that UNO acts as
	the challenger. As challenger, it generates a random hash and sends it to the other UNO, which generates a random number in
	return. The two numbers are or'ed together to decide which UNO goes first, and the accepting UNO has a chance to verify the
	other UNO is not cheating. Then, the UNO's alternate between attacking and defending until one wins.

