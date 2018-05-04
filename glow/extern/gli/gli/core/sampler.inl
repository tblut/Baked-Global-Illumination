#include <glm/gtx/wrap.hpp>

namespace gli{
namespace detail
{
	template <typename T>
	inline T passThrought(T const & SampleCoord)
	{
		return SampleCoord;
	}
}//namespace detail

	inline sampler::sampler(wrap Wrap, filter Mip, filter Min)
		: Wrap(getFunc(Wrap))
		, Mip(Mip)
		, Min(Min)
	{}

	inline sampler::wrap_type sampler::getFunc(wrap WrapMode) const
	{
		static wrap_type Table[] =
		{
			glm::clamp,
			detail::passThrought,
			glm::repeat,
			glm::mirrorRepeat,
			glm::mirrorClamp,
			glm::mirrorClamp
		};
		static_assert(sizeof(Table) / sizeof(Table[0]) == WRAP_COUNT, "Table needs to be updated");

		return Table[WrapMode];
	}
}//namespace gli
