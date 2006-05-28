#ifndef _TJCORE_H
#define _TJCORE_H

#pragma warning(push)
#pragma warning(disable: 4251)

class EXPORTED RunnableApplication {
	public:
		virtual ~RunnableApplication();
		virtual void Message(MSG& msg) = 0;
		virtual void Initialize(ref<Arguments> args) = 0;
};

class EXPORTED Runnable {
	public:
		virtual ~Runnable();
		virtual void Run() = 0;
};

class EXPORTED Core {
	public:
		static ref<Core> Instance();
		void Run(RunnableApplication* app, ref<Arguments> args);
		RunnableApplication* GetApplicationPointer();
		Core();
		virtual ~Core();
		void AddAction(ref<Runnable> action);

	protected:
		static ref<Core> _instance;
		RunnableApplication* _app;
		CriticalSection _actionLock;
		std::vector< ref<Runnable> > _actions;
		Event _actionEvent;

		void ProcessActions();
};

#pragma warning(pop)

#endif