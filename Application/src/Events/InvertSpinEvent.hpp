#pragma once

#include "CustomEventType.hpp"

class InvertSpinEvent : public mtd::CustomEvent
{
	public:
		virtual uint64_t getID() const override
		{
			return CustomEventType::InvertSpin;
		}
};
