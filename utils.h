#ifndef MOUSE_H_
#define MOUSE_H_

class Mouse {

public:
	int x;											// current mouse x
	int y;											// current mouse y
	int last_x;										// previous mouse position x
	int last_y;										// previous mouse position y

	int button;
	int state;

	bool active;

	Mouse() {
		x = 0;
		y = 0;
		last_x = 0;
		last_y = 0;
		button = 0;
		state = 0;
		active = false;
	}

	void set(int new_x, int new_y) {
		last_x = x;
		last_y = y;
		x = new_x;
		y = new_y;
	}

	int dx() {
		return x - last_x;
	}

	int dy() {
		return y - last_y;
	}

};

// convert int to string type
string int_to_string(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

#endif /* MOUSE_H_ */
