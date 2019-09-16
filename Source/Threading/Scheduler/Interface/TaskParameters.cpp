#include "TaskParameters.h"

TaskParameters::TaskParameters(bool post,
	TaskPriority priority,
	bool shouldBlock,
	int requiredThread):
	post_(post),
	priority_(priority),
	shouldBlock_(shouldBlock),
	requiredThread_(requiredThread)
{

	if (!post_) {
		shouldBlock_ = true;
		requiredThread_ = -1; //if not post, ignored in scheduler
		priority_ = TaskPriority::HIGH;
	}

}