namespace Engine {
namespace Input {
	
using EventHandlerFunc = std::function<void(const SDL_Event&)>;

struct EventManager {
	
	void pollAndHandle() const {
		SDL_Event e;
		while (SDL_PollEvent(&e)){
			const SDL_EventType type = static_cast<SDL_EventType>(e.type);
			if(handlers.count(type)) {
				handlers.at(type)(e);
			}
		}
	} 
	
	void waitAndHandle() const {
		SDL_Event e;
		SDL_WaitEvent(&e);
		const SDL_EventType type = static_cast<SDL_EventType>(e.type);
		if(handlers.count(type)) {
			handlers.at(type)(e);
		}
	}
	
	std::map<SDL_EventType, EventHandlerFunc> handlers;
};

}
}
