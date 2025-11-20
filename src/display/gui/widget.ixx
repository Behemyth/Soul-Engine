export module synodic.soul.gui:widget;

import synodic.soul.core;

import std;

export class Widget : public Component{

public:

	Widget() = default;
	virtual ~Widget() = default;


protected:

	//sub-pixel values
	// TODO: Replace with proper vector types (glm removed)
	// glm::dvec2 size_;
	// glm::dvec2 position_; //upper left position
	struct { double x, y; } size_;
	struct { double x, y; } position_; //upper left position

	//Gets
	bool DirtyFlag() const; //flag 0

	//Sets
	void DirtyFlag(bool);


private:

	std::bitset<1> flags_;


};
