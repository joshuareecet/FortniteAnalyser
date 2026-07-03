#include <chrono>

namespace helpers{
	class Timer{
		private:
			std::chrono::steady_clock::time_point initial_time;		
		public:
			void start(){
				initial_time = std::chrono::steady_clock::now();
			}
			std::chrono::nanoseconds elapsed_time(){
				auto elapsed = std::chrono::nanoseconds(std::chrono::steady_clock::now() - initial_time);
				return elapsed;
			};
			std::chrono::milliseconds elapsed_time_ms(){
				auto elapsed = std::chrono::steady_clock::now() - initial_time;
				return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
			}
			std::chrono::microseconds elapsed_time_micros(){
				auto elapsed = std::chrono::steady_clock::now() - initial_time;
				return std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
			}
			void restart(){
				initial_time = std::chrono::steady_clock::now();
			}
	};
};