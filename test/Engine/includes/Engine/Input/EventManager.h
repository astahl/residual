namespace Engine {
namespace Input {
	
using EventHandlerFunc = std::function<void(const SDL_Event&)>;

class EventManager {
public:
	void setHandler(SDL_EventType type, EventHandlerFunc handler) {
		if(handler) {
			handlers[type] = handler;
		}
		else {
			handlers.erase(type);
		}
	}
	
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
	
private:
	std::map<SDL_EventType, EventHandlerFunc> handlers;
};

}
}
