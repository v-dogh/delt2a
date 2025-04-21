#ifndef D2_BOX_HPP
#define D2_BOX_HPP

#include "../d2_tree_element.hpp"
#include "../d2_styles.hpp"
#include "d2_element_utils.hpp"

namespace d2::dx
{
	class Box :
		public VecParentElement,
		public style::UAI<Box, style::ILayout, style::IContainer, style::IColors>,
		public impl::UnitUpdateHelper<Box>,
		public impl::ContainerHelper<Box>
	{
	public:
		friend class UnitUpdateHelper;
		friend class ContainerHelper;
		using data = style::UAI<Box, style::ILayout, style::IContainer, style::IColors>;
	protected:
		void _recompute_layout(int basex = 0, int basey = 0) const noexcept
		{
			// FleshBox Spatial Relativity Protocol (FB-SRP)
			// Division into virtual groups
			// Each sequence of elements with the same direction of alignment (vertical/horizontal)
			// belongs to the same virtual group.
			// The virtual group's alignment is the alignment of the next group
			// If we have a block with alignment A and the last element has alignment A and B then we introduce a break.
			// We proceed as usual by constructing a new block, but we only reset the offset corresponding to the previous
			// block's alignment
			// Examples:
			// Child 1,2 - horizontal
			// Child 3 - vertical
			// Child 4 - horizontal
			// Virtual Groups: {{{ 1H, 2H }V, 3V }H 4H }
			// Horizontal group
			// A - X
			// B - X
			// Break / horizontal group (carries from previous group)
			// C - X,Y
			// D - X
			// Result
			// AB
			// CD
			//
			// Size pooling
			// Each virtual group which contains an element with a relative size is marked as volatile.
			// If the relative size is aligned with it's positioning, the group receives a 'pool' of size
			// which it will allocate equally between it's elements (that have relative sizes on this axis).
			// Elements with absolute dimensions will substract their sizes from the pool.
			// If the relative size is not aligned with it's positioning, the entire group will be subject to this procedure
			// with respect to the rest of the groups on this axis.
			// Examples:
			// Q - X
			// A - XW
			// B - XW
			// C - XYW
			// D - XW
			// Layout:
			// Q(Q_width) A((width - Q_width) / 2) B((width - Q_width) / 2)
			// C(width / 2) C(width / 2)
			//
			// Q - Y
			// A - XYWH
			// B - XWH
			// C - XYWH
			// D - XWH
			// Layout:
			// Q(*, Q_height)
			// A(*, (height - Q_height) / 2) B(*, (height - Q_height) / 2)
			// C(*, (height - Q_height) / 2) D(*, (height - Q_height) / 2)

			int cxpos = 0;
			int cypos = 0;
			bool relh = false;
			bool relv = false;

			for (std::size_t i = 0; i < _elements.size();)
			{
				auto& elem = _elements[i];
				if (!elem->getstate(Display))
				{
					i++;
					continue;
				}

				const auto relrep = elem->unit_report();
				const bool relht = relrep & RelativeXPos;
				const bool relvt = relrep & RelativeYPos;

				// Break the block if we had an opposite flag on the last element

				if (relht && relvt)
				{
					if (relh)
					{
						relh = true;
						relv = false;
					}
					else if (relv)
					{
						relv = true;
						relh = false;
					}
				}
				else
				{
					relh = relht;
					relv = relvt;
				}

				if (relh || relv)
				{
					// Calculate group

					auto beg = i;
					int reldims_pool = 0;
					int relcount = 0;
					bool is_pooled = false;

					int cxpos_offset = 0;
					int cypos_offset = 0;
					bool first = true;
					bool resetx = false;
					bool resety = false;

					do
					{
						auto sub = _elements[i];
						const auto relreps = sub->unit_report();
						const bool relhs = relreps & RelativeXPos;
						const bool relvs = relreps & RelativeYPos;

						// Absolute object (skip)
						if (!(relhs || relvs) ||
							!sub->getstate(State::Display))
						{
							i++;
							continue;
						}
						// Break block
						else if (
							!first &&
							((relh && !relhs) ||
							(relv && !relvs) ||
							(relv && relhs) ||
							(relh && relvs))
							)
						{

							if (relv && relhs) resety = true;
							else if (relh && relvs) resetx = true;
							break;
						}

						// Relative size metadata

						const auto bbox = sub->internal_box();

						if ((relreps & RelativeWidth) && relhs)
						{
							if (!is_pooled)
							{
								reldims_pool += box().width;
								is_pooled = true;
							}
							relcount++;
						}
						if (relhs)
						{
							reldims_pool -= bbox.width;
						}

						if ((relreps & RelativeHeight) && relvs)
						{
							if (!is_pooled)
							{
								reldims_pool += box().height;
								is_pooled = true;
							}
							relcount++;
						}
						if (relvs)
						{
							reldims_pool -= bbox.height;
						}

						first = false;
						i++;
					} while (i < _elements.size());

					const int rel_size = relcount ? reldims_pool / relcount : 0;

					do
					{
						auto sub = _elements[beg];
						const auto relreps = sub->unit_report();
						const bool relhs = relreps & RelativeXPos;
						const bool relvs = relreps & RelativeYPos;

						if (!(relhs || relvs) ||
							!sub->getstate(State::Display))
						{
							beg++;
							continue;
						}

						const auto bbox = sub->internal_box();
						const auto ipos = sub->internal_position();
						const auto pos = first ? Position{
							relhs ? (_resolve_units_impl(Unit(ipos.x, Unit::Px), sub) + basex) : ipos.x,
							relvs ? (_resolve_units_impl(Unit(ipos.y, Unit::Px), sub) + basey) : ipos.y
						} : ipos;
						sub->override_position({
							pos.x + cxpos,
							pos.y + cypos
						});

						if (relv)
						{
							const auto rwidth = bbox.width;
							const auto rheight = rel_size + bbox.height;

							cypos += rheight + pos.y;
							cxpos_offset = std::max(
								cxpos_offset,
								rwidth + pos.x
							);

							if (relreps & RelativeHeight)
							{
								sub->override_dimensions({
									rwidth, rheight
								});
							}
							else
							{
								sub->override_dimensions({
									bbox.width, bbox.height
								});
							}
						}
						else
						{
							const auto rwidth = rel_size + bbox.width;
							const auto rheight = bbox.height;

							cxpos += rwidth + pos.x;
							cypos_offset = std::max(
								cypos_offset,
								rheight + pos.y
							);

							if (relreps & RelativeWidth)
							{
								sub->override_dimensions({
									rwidth, rheight
								});
							}
							else
							{
								sub->override_dimensions({
									bbox.width, bbox.height
								});
							}
						}

						first = false;
						beg++;
					} while (beg != i);

					if (resetx) cxpos = 0;
					else if (resety) cypos = 0;

					cxpos += cxpos_offset;
					cypos += cypos_offset;
				}
				else
					i++;
			}
		}
		bool _is_automatic_layout() const noexcept
		{
			return !(data::container_options & ContainerOptions::DisableAutoLayout);
		}

		virtual Position _position_for(cptr elem) const override
		{
			if (elem->relative_position())
			{
				_recompute_layout();
				return elem->position();
			}
			return elem->internal_position();
		}
		virtual BoundingBox _dimensions_for(cptr elem) const override
		{
			if (elem->relative_dimensions())
			{
				_recompute_layout();
				return elem->box();
			}
			return elem->internal_box();
		}

		virtual bool _provides_layout_impl(cptr elem) const noexcept override
		{
			return elem->relative_position() && _is_automatic_layout() && elem->getstate(Display);
		}
		virtual bool _provides_dimensions_impl(cptr elem) const noexcept override
		{
			return elem->relative_dimensions() && _is_automatic_layout();
		}

		virtual int _resolve_units_impl(Unit value, cptr elem) const noexcept override
		{
			if (!(value.getmods() & Unit::Relative) || !_is_automatic_layout())
				return ParentElement::_resolve_units_impl(value, elem);
			return value.raw();
		}
		virtual int _get_border_impl(BorderType type, cptr elem) const noexcept override
		{
			if (
				(data::container_options & ContainerOptions::EnableBorder) &&
				elem->getzindex() < 6
				)
				return _resolve_units(data::border_width);
			return 0;
		}
		virtual char _index_impl() const noexcept override
		{
			return data::zindex;
		}
		virtual unit_meta_flag _unit_report_impl() const noexcept override
		{
			return UnitUpdateHelper::_report_units();
		}

		virtual BoundingBox _dimensions_impl() const noexcept override
		{
			if (data::width.getunits() == Unit::Auto || data::height.getunits() == Unit::Auto)
			{
				const auto bw = (data::container_options & ContainerOptions::EnableBorder) ?
					_resolve_units(data::border_width) : 0;

				int perfect_width = 0;
				int perfect_height = 0;
				for (decltype(auto) it : _elements)
				{			
					if (it->contextual_dimensions())
					{
						continue;
					}
					else if (it->contextual_position())
					{
						const auto bbox = it->box();
						perfect_width = std::max(perfect_width, bbox.width);
						perfect_height = std::max(perfect_height, bbox.height);
					}
					else
					{
						const auto [ xpos, ypos ] = it->position();
						const auto [ width, height ] = it->box();
						perfect_width = std::max(perfect_width, xpos + width);
						perfect_height = std::max(perfect_height, ypos + height);
					}
				}

				BoundingBox res;
				if (data::width.getunits() == Unit::Auto)
					res.width = perfect_width + (bw * 2) + data::width.raw();
				else res.width = _resolve_units(data::width);
				if (data::height.getunits() == Unit::Auto)
					res.height = perfect_height + (bw * 2) + data::height.raw();
				else res.height = _resolve_units(data::height);
				return res;
			}
			else
			{
				return {
					_resolve_units(data::width),
					_resolve_units(data::height)
				};
			}
		}
		virtual Position _position_impl() const noexcept override
		{
			return {
				_resolve_units(data::x),
				_resolve_units(data::y)
			};
		}

		virtual void _frame_impl(PixelBuffer::View buffer) noexcept override
		{
			// Fill in the gaps

			buffer.fill(Pixel::combine(data::foreground_color, data::background_color));

			// Fleshbox

			std::vector<Element*> ptrs{};
			std::size_t idx = 0;
			{
				ptrs.reserve(_elements.size());
				foreach_internal([&ptrs](ptr ptr) { ptrs.push_back(ptr.get()); });
				std::sort(ptrs.begin(), ptrs.end(), [](const auto& a, const auto& b) { return a->getzindex() < b->getzindex(); });

				_fleshbox_render_start();

				// Border is zindex = 6 (relative) so we defer the rest to after we render the border
				while (idx < ptrs.size() && ptrs[idx]->getzindex() < 6)
				{
					auto& obj = *ptrs[idx];
					if (obj.getstate(Display))
						_fleshbox_object_render(buffer, obj.shared_from_this());
					idx++;
				}
			}

			ContainerHelper::_render_border(buffer);

			// Render the rest of the objects
			{
				while (idx < ptrs.size())
				{
					auto& obj = *ptrs[idx];
					if (obj.getstate(Display))
						_fleshbox_object_render(buffer, obj.shared_from_this());
					idx++;
				}
			}
		}

		// Fleshbox protocol

		virtual void _fleshbox_render_start() {}
		virtual void _fleshbox_object_render(PixelBuffer::View buffer, ptr obj)
		{
			// Mezon the monstrous
			// It is high time
			const auto [ x, y ] = obj->position();
			const auto [ width, height ] = obj->box();
			const auto w = static_cast<int>(buffer.width());
			const auto h = static_cast<int>(buffer.height());

			if (x - width <= w &&
				y - height <= h)
			{
				const auto f = obj->frame();
				buffer.inscribe(x, y, f.buffer());
			}
		}
	public:
		using VecParentElement::VecParentElement;
	};
}

#endif // D2_BOX_HPP
