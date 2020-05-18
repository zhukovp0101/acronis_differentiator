#include <cstdlib>
#include <memory>

#include <Vector.h>
#include "gtest/gtest.h"

#include "Helper.h"

TEST(VectorTests, emplace_back) {
	const size_t kIterations = 123456;

	Vector<Helper> vector;
	for (size_t i = 0; i < kIterations; ++i) {
		vector.emplace_back(i, i, i);
	}

	for (size_t i = 0; i < kIterations; ++i) {
		ASSERT_EQ(i, vector[i].a_);
		ASSERT_EQ(i, vector[i].b_);
		ASSERT_EQ(i, vector[i].c_);
	}
}

TEST(VectorTests, push_and_pop_back) {
	const size_t kIterations = 123456;

	Vector<Helper> vector;
	for (size_t i = 0; i < kIterations; ++i) {
		vector.push_back(Helper(i, i, i));
	}

	for (size_t i = 0; i < kIterations; ++i) {
		ASSERT_EQ(i, vector[i].a_);
		ASSERT_EQ(i, vector[i].b_);
		ASSERT_EQ(i, vector[i].c_);
	}

	for (size_t i = kIterations; i > 0; --i) {
		ASSERT_EQ(vector.back().a_, i - 1);
		vector.pop_back();
	}

	ASSERT_TRUE(vector.empty());
}

TEST(VectorTests, size_and_empty) {
	const size_t kIterations = 123456;

	Vector<Helper> vector;
	ASSERT_TRUE(vector.empty());
	for (size_t i = 0; i < kIterations; ++i) {
		vector.push_back(Helper(i, i, i));
		ASSERT_EQ(i + 1, vector.size());
		ASSERT_FALSE(vector.empty());
	}
}

TEST(VectorTests, erase) {
	const size_t kIterations = 12345;

	Vector<Helper> vector;
	for (size_t i = 0; i < kIterations; ++i) {
		vector.emplace_back(i, i, i);
	}

	auto destroy_order = std::vector<size_t>();
	Helper::destroy_order_ = std::make_shared<std::vector<size_t>>();

	srand(time(nullptr));

	for (size_t i = 0; i < kIterations; ++i) {
		size_t cur_width = std::max(rand() % vector.size(), size_t(1));
		size_t cur_pos = rand() % (vector.size() + 1 - cur_width);
		for (size_t j = 0; j < cur_width; ++j) {
			destroy_order.push_back(cur_pos + j);
		}
		vector.erase(vector.begin() + cur_pos, vector.begin() + cur_pos + cur_width);
		if (vector.empty()) {
			break;
		}
 	}

	ASSERT_TRUE(vector.empty());

	std::vector<size_t> usual_order;
	for (size_t i = 0; i < kIterations; ++i) {
		usual_order.emplace_back(i);
	}
	std::sort(Helper::destroy_order_->begin(), Helper::destroy_order_->end());
	ASSERT_EQ(*Helper::destroy_order_, usual_order);
}

TEST(VectorTests, resize_reserve) {
	const size_t kIterations = 12345;

	Vector<Helper> vector;
	vector.resize(kIterations);

	for (size_t i = 0; i < kIterations; ++i) {
		ASSERT_EQ(vector[i].a_, 0);
	}

	vector.resize(0);
	ASSERT_TRUE(vector.empty());

	vector.reserve(kIterations);
	Helper::move_counter_ = 0;

	for (size_t i = 0; i < kIterations; ++i) {
		vector.push_back(Helper(i, i, i));
	}

	ASSERT_EQ(Helper::move_counter_, kIterations);
}

TEST(VectorTests, InitializerList) {
	Vector<Helper> vector = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}, {5, 5, 5}, {6, 6, 6}, {7, 7, 7}};
	std::vector<Helper> std_vector = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}, {5, 5, 5}, {6, 6, 6}, {7, 7, 7}};

	for (size_t i = 0; i < 8; ++i) {
		ASSERT_EQ(vector[i].c_, std_vector[i].c_);
	}
}
