#pragma once

template<typename Callback>
//	requires callable<Callback>
class ScopeGuard final
{
public:
	explicit ScopeGuard(Callback callback)
		: _callback{ callback }
	{
	}

	~ScopeGuard()
	{
		_callback();
	}

	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard& operator=(const ScopeGuard&) = delete;

	ScopeGuard(ScopeGuard&&) = delete;
	ScopeGuard& operator=(ScopeGuard&&) = delete;

private:
	Callback _callback;
};
