#ifndef D2_EXCEPTIONS_HPP
#define D2_EXCEPTIONS_HPP

#include <exception>
#include <format>
#include <string>

namespace d2
{
#	define D2_EXPECT(...) (((__VA_ARGS__) == false) ? std::terminate() : void());
#	ifndef NDEBUG
#	define D2_ASSERT(...) D2_EXPECT(__VA_ARGS__);
#	else
#	define D2_ASSERT(...) void();
#	endif

#	define D2_EXCEPTION(name, log) \
		struct name : Exception { name() : Exception(log) { } };
#	define D2_GEN_EXCEPTION(name, pre) \
		struct name : Exception \
		{ \
			name(const std::string& error) \
				: Exception(std::format(pre": {}", error)) {} \
		}; \

	struct Exception : std::exception
	{
	private:
		const std::string error_{};
	public:
		Exception(const std::string& error)
			: error_(error) {}

        virtual const char* what() const noexcept override final
		{
			return error_.c_str();
		}
	};

	D2_EXCEPTION(ScreenStartException, "Attempt to start the screen frame loop when one is already running");

	D2_GEN_EXCEPTION(GenericElementException, "Generic exception thrown by element");
	D2_EXCEPTION(CastException, "Access of element through invalid type");

	D2_EXCEPTION(ChildRemovalException, "Child does not exist for removal");
	D2_EXCEPTION(ChildCreationException, "Child already exists, cannot overwrite");
	D2_EXCEPTION(InvalidChildReferenceException, "Attempt to access non-existent child");

	D2_GEN_EXCEPTION(IOContextException, "Generic exception thrown by IO context");
	D2_EXCEPTION(EventIndexException, "Invalid event index provided to IO context");
	D2_EXCEPTION(EventOverwriteException, "Attempt to override active listener handle");
} // d2

#endif // D2_EXCEPTIONS_HPP
