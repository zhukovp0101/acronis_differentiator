#pragma once

#include <memory>
#include <vector>

class Helper {
 public:
	Helper() = default;
	~Helper() {
		destroy_order_->push_back(a_);
	}
	Helper(int a, int b, int c) : a_(a), b_(b), c_(c) {
	}

	Helper(Helper &&another) {
		++move_counter_;
		a_ = another.a_;
		b_ = another.b_;
		c_ = another.c_;
	}

	Helper(const Helper &another) {
		a_ = another.a_;
		b_ = another.b_;
		c_ = another.c_;
	}

	Helper& operator =(const Helper &another) {
		a_ = another.a_;
		b_ = another.b_;
		c_ = another.c_;
		return *this;
	}

	static std::shared_ptr<std::vector<size_t>> destroy_order_;
	static size_t move_counter_;
	int a_ = 0;
	int b_ = 0;
	int c_ = 0;
};


