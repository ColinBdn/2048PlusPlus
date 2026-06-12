#pragma once
#include <iostream>
#include <vector>
#include "../../glHelper/glHelper.h"

class Spb
{
public:
	void startGameStandalone();
	void reset();
	int left(float a);
	int right(float a);
	void startDrawing();
	void stopDrawing();
	void drawFrame(bool* p_open);
	void updatePhysics();
	void drawCart();
	void drawPole();
	void showPhysicData();
	void updateScore();

	Spb();
	~Spb();

	static float genRandomFloat(const float& a, const float& b);
	static int genRandomInt(int a, int b);
	static bool proba(float proba);

	struct Vector2
	{
		float x;
		float y;
		operator std::pair<float, float>() const
		{
			return std::make_pair(x, y);
		}
		operator ImVec2() const {
			return ImVec2(x, y);
		}
		Vector2() : x(0), y(0) {}
		Vector2(float x_, float y_) : x(x_), y(y_) {}
		Vector2(const ImVec2& vec) : x((float)vec.x), y((float)vec.y) {}
	};

	// 1m = 100px
	struct Pole
	{
		float mass;
		float length;
		float angle;
		float velocity;
	};

	struct Cart
	{
		Vector2 dim;
		float wheelRadius;
		Vector2 pos;
		Vector2 velocity;
		Vector2 acceleration;
		bool is_moving;
	};

	Pole m_pole;
	Cart m_cart;
	bool m_firstPrint;
	long long m_deltaTime;
	float m_acceleration_force;
	float m_maxSpeed;
	int m_frame;
	float m_score;
	bool m_isVertical;
	float m_limit;
};