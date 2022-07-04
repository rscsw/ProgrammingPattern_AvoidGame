#pragma once
#include <OpenGL.hpp>

using namespace std;

namespace AvoidGame
{
	class GameLoop
	{
	private:
		bool _isGameRunning;

	public:
		GameLoop() { _isGameRunning = false; }
		~GameLoop() {}

		void Run()
		{
			_isGameRunning = true;
			Initialize();

			while (_isGameRunning)
			{

				Input();
				Update();
				Render();

			}
			Release();
		}
		void Stop()
		{
			_isGameRunning = false;
		}

	private:
		void Initialize()
		{
			
		}
		void Release()
		{
		}

		void Input()
		{
			
		}
		void Update()
		{

		}
		void Render()
		{
			
		}
	};
}