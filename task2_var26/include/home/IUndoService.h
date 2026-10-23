#pragma once


namespace smarthome::home
{
	class IUndoService
	{
	public:
		virtual ~IUndoService() = default;
		virtual bool undoLast() = 0;
	};
}