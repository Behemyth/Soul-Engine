#pragma once

#include "glm/glm.hpp"
#include "Core/Composition/Component/Component.h"

#include <bitset>


class Widget : public Component{

public:

	Widget() = default;
	virtual ~Widget() = default;


protected:

	//sub-pixel values
	glm::dvec2 size_;
	glm::dvec2 position_; //upper left position

	//Gets
	bool DirtyFlag() const; //flag 0

	//Sets
	void DirtyFlag(bool);


private:

	std::bitset<1> flags_;


};

