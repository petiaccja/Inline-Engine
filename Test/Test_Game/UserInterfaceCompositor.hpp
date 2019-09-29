#pragma once

#include "WindowLayout.hpp"

#include "GuiEngine/Board.hpp"
#include "GuiEngine/Frame.hpp"

#include <type_traits>


class UserInterfaceCompositor;

class TopLevelFrame {
	friend class UserInterfaceCompositor;

public:
	virtual ~TopLevelFrame() = default;

protected:
	UserInterfaceCompositor& GetCompositor() const;
	WindowLayout::Binding& GetBinding() const;

private:
	void SetCompositor(UserInterfaceCompositor* compositor);

private:
	UserInterfaceCompositor* m_compositor = nullptr;
};


class UserInterfaceCompositor {
	template <class FrameT>
	static constexpr bool frame_type_valid = std::is_base_of_v<inl::gui::Control, FrameT>&& std::is_base_of_v<TopLevelFrame, FrameT>;

public:
	UserInterfaceCompositor(inl::gui::Board& board);

	void SetPosition(inl::Vec2 pos);
	void SetSize(inl::Vec2 size);
	inl::Vec2 GetPosition() const;
	inl::Vec2 GetSize() const;

	template <class FrameT, class... Args>
	std::enable_if_t<frame_type_valid<FrameT>, FrameT&> ShowFrame(Args&&... args);

	template <class FrameT>
	std::enable_if_t<frame_type_valid<FrameT>> HideFrame();

	template <class FrameT>
	std::enable_if_t<frame_type_valid<FrameT>> DeleteFrame();

	WindowLayout::Binding& GetBinding(const TopLevelFrame& frame);

	template <class FrameT>
	WindowLayout::Binding& GetBinding();

	template <class FrameT>
	FrameT& GetFrame();

private:
	template <class FrameT>
	std::optional<std::reference_wrapper<FrameT>> FindFrame();

	template <class FrameT>
	std::optional<std::reference_wrapper<const FrameT>> FindFrame() const;

private:
	inl::gui::Board& m_board;
	std::vector<std::shared_ptr<TopLevelFrame>> m_frames;
	WindowLayout m_layout;
};


template <class FrameT, class... Args>
std::enable_if_t<UserInterfaceCompositor::frame_type_valid<FrameT>, FrameT&> UserInterfaceCompositor::ShowFrame(Args&&... args) {
	auto existingFrame = FindFrame<FrameT>();
	if (existingFrame) {
		auto* control = dynamic_cast<inl::gui::Control*>(&existingFrame.value().get());
		assert(control);
		control->SetVisible(true);
		return existingFrame.value();
	}
	else {
		auto frame = std::make_shared<FrameT>(std::forward<Args>(args)...);
		m_frames.push_back(frame);
		m_layout.AddChild(frame);
		frame->SetCompositor(this);
		return *frame;
	}
}


template <class FrameT>
std::enable_if_t<UserInterfaceCompositor::frame_type_valid<FrameT>> UserInterfaceCompositor::HideFrame() {
	auto existingFrame = FindFrame<FrameT>();
	if (existingFrame) {
		auto* control = dynamic_cast<inl::gui::Control*>(&existingFrame.value().get());
		assert(control);
		control->SetVisible(false);
	}
}


template <class FrameT>
std::enable_if_t<UserInterfaceCompositor::frame_type_valid<FrameT>> UserInterfaceCompositor::DeleteFrame() {
	auto existingFrame = FindFrame<FrameT>();
	if (existingFrame) {
		auto* control = dynamic_cast<inl::gui::Control*>(&existingFrame.value().get());
		assert(control);
		m_layout.RemoveChild(control);
		auto it = std::find_if(m_frames.begin(), m_frames.end(), [&existingFrame](auto& sptr) { return sptr.get() == &existingFrame.value().get(); });
		m_frames.erase(it);
	}
}


template <class FrameT>
WindowLayout::Binding& UserInterfaceCompositor::GetBinding() {
	return GetBinding(GetFrame<FrameT>());
}


template <class FrameT>
FrameT& UserInterfaceCompositor::GetFrame() {
	auto existingFrame = FindFrame<FrameT>();
	if (!existingFrame) {
		throw inl::OutOfRangeException("Frame not found.");
	}
	return existingFrame.value();
}


template <class FrameT>
std::optional<std::reference_wrapper<FrameT>> UserInterfaceCompositor::FindFrame() {
	for (auto& frame : m_frames) {
		FrameT* ptr = dynamic_cast<FrameT*>(frame.get());
		if (ptr) {
			return std::ref(*ptr);
		}
	}
	return {};
}


template <class FrameT>
std::optional<std::reference_wrapper<const FrameT>> UserInterfaceCompositor::FindFrame() const {
	for (auto& frame : m_frames) {
		FrameT* ptr = dynamic_cast<FrameT*>(frame.get());
		if (ptr) {
			return std::ref(*ptr);
		}
	}
	return {};
}
