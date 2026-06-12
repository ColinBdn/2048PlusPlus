#include <random>
#include <string>
#include <chrono>
#include <thread>
#include <cmath>

#include "keyboard.h"
#include "singlePoleBalancing.h"


long long getCurrentTime() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


void Spb::startGameStandalone()
{
    const std::chrono::nanoseconds targetUpdateDuration = std::chrono::nanoseconds((int)round(1000000000.0/60.0));
	std::chrono::high_resolution_clock::time_point startTime, endTime, currentTime, nextUpdateTime;
	std::chrono::high_resolution_clock::duration updateDuration, remainingTime, elapsedTime;

	reset();
	startDrawing();

	while (true)
	{
		startTime = std::chrono::high_resolution_clock::now();
		m_frame++;

		m_cart.is_moving = false;
		if (Keyboard::isKeyDown('Q'))
		{
			left(m_acceleration_force);
		}
		if (Keyboard::isKeyDown('D'))
		{
			right(m_acceleration_force);
		}


		updatePhysics();
		updateScore();

		if (abs(m_pole.angle) < (3.14 / 6))
		{
			m_score += 2;
		}
		else if (abs(m_pole.angle) < (3.14 / 3))
		{
			m_score += 0.5;
		}

		do
		{
			endTime = std::chrono::high_resolution_clock::now();
			updateDuration = endTime - startTime;
			remainingTime = targetUpdateDuration - updateDuration;
		}
		while (remainingTime >= std::chrono::nanoseconds(0));

		if (updateDuration > targetUpdateDuration*1.25)
		{
			std::cout << "Warning: Update took much longer than target duration.\n";
		}
		endTime = std::chrono::high_resolution_clock::now();
		m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	}
}

void Spb::reset()
{
	m_frame = 0;
	m_deltaTime = 0;
	m_score = 1;
	m_isVertical = false;

	m_cart.pos = {0,0};
	m_cart.dim = { 1.3, 0.6 };
	m_cart.wheelRadius = 0.15;
	m_cart.velocity = { 0,0 };
	m_cart.acceleration = { 0,0 };

	m_pole.mass = 8;
	m_pole.length = 3;
	m_pole.angle = 3.14;
	m_pole.velocity = 0;
}

int Spb::left(float a)
{
	if (abs(m_cart.pos.x) < m_limit)
	{
		m_cart.is_moving = true;
		if (abs(m_cart.velocity.x) < m_maxSpeed)
		{
			m_cart.acceleration.x = -a;
		}
		return 0;
	}
	else
	{
		return 1;
	}
}

int Spb::right(float a)
{
	if (abs(m_cart.pos.x) < m_limit)
	{
		m_cart.is_moving = true;
		if (m_cart.velocity.x < m_maxSpeed)
		{
			m_cart.acceleration.x = a;
		}
		return 0;
	}
	else
	{
		return 1;
	}
}

void Spb::startDrawing()
{
	GlHelper::add(&Spb::drawFrame, this, "spbFrame");
}

void Spb::stopDrawing()
{
	GlHelper::remove("spbFrame", this);
}

void Spb::drawFrame(bool *p_open)
{
	ImGuiWindowClass window_class;
	window_class.ViewportFlagsOverrideSet = ImGuiViewportFlags_NoAutoMerge;
	ImGui::SetNextWindowClass(&window_class);


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vector2(0.0f, 0.0f));
	ImGui::Begin(("Spb##" + std::to_string(reinterpret_cast<uintptr_t>(this))).c_str(), p_open);
	if (m_firstPrint)
	{
		ImGui::SetWindowSize(Vector2(1400, 800));
		m_firstPrint = false;
	}

	ImGui::Text("graphic average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("physic average %.3f ms/frame (%.1f FPS)", m_deltaTime, 1000.0f / m_deltaTime);

	drawCart();
	drawPole();
	showPhysicData();

	ImGui::PopStyleVar();
	ImGui::End();
}

void Spb::drawCart()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	Vector2 winTopLeft = ImGui::GetCursorScreenPos();
	Vector2 center = { winTopLeft.x + ImGui::GetWindowWidth() / 2.f , winTopLeft.y + (ImGui::GetWindowHeight() + ImGui::GetTextLineHeightWithSpacing()) / 2.f };
	Vector2 windowsSize = { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 4 };

	unsigned int color_white = IM_COL32(255, 255, 255, 255);
	float groundPosY = winTopLeft.y + 2.f * windowsSize.y / 3.f;
	draw_list->AddLine({ winTopLeft.x, groundPosY }, { winTopLeft.x + windowsSize.x, groundPosY }, color_white, 3);

	Vector2 cartDrawPos = { m_cart.pos.x*100+center.x, m_cart.pos.y*100 + groundPosY - m_cart.wheelRadius*100*2 - m_cart.dim.y*100/2};

	Vector2 topLeft = { cartDrawPos.x - m_cart.dim.x*100 / 2 , cartDrawPos.y - m_cart.dim.y * 100 / 2 };
	Vector2 topRight = { cartDrawPos.x + m_cart.dim.x * 100 / 2 , cartDrawPos.y - m_cart.dim.y * 100 / 2 };
	Vector2 bottomLeft = { cartDrawPos.x - m_cart.dim.x * 100 / 2 , cartDrawPos.y + m_cart.dim.y * 100 / 2 };
	Vector2 bottomRight = { cartDrawPos.x + m_cart.dim.x * 100 / 2 , cartDrawPos.y + m_cart.dim.y * 100 / 2 };

	draw_list->AddLine(topLeft, topRight, color_white, 4);
	draw_list->AddLine(bottomLeft, bottomRight, color_white, 4);
	draw_list->AddLine(bottomLeft, topLeft, color_white, 4);
	draw_list->AddLine(bottomRight, topRight, color_white, 4);

	draw_list->AddCircle({ bottomLeft.x + m_cart.dim.x * 100 / 5, bottomLeft.y + m_cart.wheelRadius * 100 }, m_cart.wheelRadius * 100, color_white, 32, 3);
	draw_list->AddCircle({ bottomRight.x - m_cart.dim.x * 100 / 5, bottomRight.y + m_cart.wheelRadius * 100 }, m_cart.wheelRadius * 100, color_white, 32, 3);
}
void Spb::drawPole()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	Vector2 winTopLeft = ImGui::GetCursorScreenPos();
	Vector2 center = { winTopLeft.x + ImGui::GetWindowWidth() / 2.f , winTopLeft.y + (ImGui::GetWindowHeight() + ImGui::GetTextLineHeightWithSpacing()) / 2.f };
	Vector2 windowsSize = { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 4 };

	unsigned int color_white = IM_COL32(255, 255, 255, 255);
	float groundPosY = winTopLeft.y + 2.f * windowsSize.y / 3.f;
	Vector2 cartDrawPos = { m_cart.pos.x * 100 + center.x, m_cart.pos.y * 100 + groundPosY - m_cart.wheelRadius * 100 * 2 - m_cart.dim.y * 100 / 2 };

	Vector2 poleEnd;
	Vector2 poleStart;

	poleStart = cartDrawPos;
	poleEnd = { sin(m_pole.angle)*m_pole.length*100 + cartDrawPos.x, -cos(m_pole.angle)*m_pole.length*100 + cartDrawPos.y};

	draw_list->AddLine(poleStart, poleEnd, color_white, 4);
	draw_list->AddCircleFilled(poleStart, 6, color_white, 32);
}
void Spb::showPhysicData()
{
	ImGui::Text("score = %f", m_score);
	ImGui::Text("-- cart physic:");
	//ImGui::Text(("acceleration=" + std::to_string(m_cart.acceleration.x)).c_str());
	ImGui::Text("velocity = %f", m_cart.velocity.x);
	ImGui::Text("position = %f", m_cart.pos.x);
	ImGui::Text("-- pole physic:");
	ImGui::Text("velocity = %f", m_pole.velocity);
	ImGui::Text("angle = %f", m_pole.angle);
	ImGui::Text("number of frame = %d", m_frame);
	ImGui::Text("isVertical = %d", m_isVertical);
}

void Spb::updateScore()
{
	double pi = 3.14159265358979323846;
	m_score += -1/(0.4*sqrt(2*pi)) * exp(-0.5* pow((m_pole.angle-pi)/(pi/3), 2)) +1;

	//if (abs(m_cart.pos.x) > 3)
	//{
	//	m_score -= 0.1;
	//}


	if (m_pole.angle < pi/12 || m_pole.angle > 23*pi/12)
	{
		//m_score++;
		m_isVertical = true;
		m_score += 3 - m_pole.velocity;
	}


	//if (m_isVertical)
	//{
	//	m_score++;
	//}
}

Spb::Spb()
{
	m_limit = 6;
	m_firstPrint = true;
	m_deltaTime=0;
	m_acceleration_force=200;
	m_maxSpeed=20;
	m_frame = 0;
	m_score = 0;
	m_isVertical = false;

	m_cart.pos = { 0,0 };
	m_cart.dim = { 1.3, 0.6 };
	m_cart.wheelRadius = 0.15;
	m_cart.velocity = { 0,0 };
	m_cart.acceleration = { 0,0 };

	m_pole.mass = 8;
	m_pole.length = 3;
	m_pole.angle = 0;
	m_pole.velocity = 0;
}
Spb::~Spb()
{
	stopDrawing();
}


void Spb::updatePhysics()
{
	const float g = 9.80665;
	const float dt = 0.01;
	float g_div_len = g / m_pole.length;
	float sin_angle = sin(m_pole.angle);
	float cos_angle = cos(m_pole.angle);
	float acceleration = g_div_len * sin_angle * m_pole.mass - m_cart.acceleration.x / m_pole.length * cos_angle;

	acceleration -= m_pole.velocity * 0.2; // frottement
	m_pole.velocity += acceleration * dt;
	m_pole.angle += m_pole.velocity * dt;

	//if (m_pole.angle > 3.14159265358979323846)
	//{
	//	m_pole.angle = m_pole.angle - 2*3.14159265358979323846  ;
	//}
	//else if (m_pole.angle < -3.14159265358979323846)
	//{
	//	m_pole.angle = m_pole.angle + 2*3.14159265358979323846;
	//}
	if (m_pole.angle > 2*3.14159265358979323846)
	{
		m_pole.angle = m_pole.angle - 2 * 3.14159265358979323846;
	}
	else if (m_pole.angle < 0)
	{
		m_pole.angle = m_pole.angle + 2*3.14159265358979323846;
	}


	if (abs(m_cart.acceleration.x)>0)
	{
		m_cart.velocity.x += m_cart.acceleration.x * dt;
	}
	else if (m_cart.is_moving==false)
	{
		m_cart.velocity.x = 0;
	}
	m_cart.pos.x += m_cart.velocity.x * dt;
	m_cart.acceleration.x = 0;
}




float Spb::genRandomFloat(const float& a, const float& b)
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> dist(a, b);
	return dist(rng);
}
int Spb::genRandomInt(int a, int b)
{
	static std::mt19937 gen(std::random_device{}());
	std::uniform_int_distribution<int> dis(a, b);
	return dis(gen);
}
bool Spb::proba(float probability)
{
	static std::mt19937 gen(std::random_device{}());
	std::uniform_real_distribution<float> dis(0.0, 1.0);
	return dis(gen) <= probability;
}
