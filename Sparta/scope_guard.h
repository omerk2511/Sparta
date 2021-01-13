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
	auto operator=(const ScopeGuard&) -> ScopeGuard& = delete;

	ScopeGuard(ScopeGuard&&) = delete;
	auto operator=(ScopeGuard&&) -> ScopeGuard& = delete;

private:
	Callback _callback;
};
