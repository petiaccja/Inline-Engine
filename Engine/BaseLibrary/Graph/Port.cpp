#include "Port.hpp"
#include "Node.hpp"

#include <cassert>


namespace inl {


//------------------------------------------------------------------------------
// InputPortBase
//------------------------------------------------------------------------------


InputPortBase::InputPortBase() {
	link = nullptr;
}


InputPortBase::~InputPortBase() {
	Unlink();
}


void InputPortBase::Link(OutputPortBase* source) {
	if (GetLink() != nullptr) {
		throw InvalidStateException("Input port is already linked.");
	}

	// just let the source OutputPortBase do the nasty stuff
	// note that OutputPortBase is a friend, and sets this' members correctly
	return source->Link(this);
}


void InputPortBase::Unlink() {
	if (link != nullptr) {
		// note that OutputPortBase is a friend, and sets this' members correctly
		link->Unlink(this);
	}
}


OutputPortBase* InputPortBase::GetLink() const {
	return link;
}

void InputPortBase::SetLinkState(OutputPortBase* link) {
	this->link = link;
}


/// Add observer node.
/// Observers are notified when new input is set.
void InputPortBase::AddObserver(NodeBase* observer) {
	observers.insert(observer);
}

/// Remove observer.
void InputPortBase::RemoveObserver(NodeBase* observer) {
	observers.erase(observer);
}


void InputPortBase::NotifyAll() {
	for (auto v : observers) {
		v->Notify(this);
	}
}




//------------------------------------------------------------------------------
// OutputPortBase
//
//------------------------------------------------------------------------------

OutputPortBase::OutputPortBase() {
	// = default
}


OutputPortBase::~OutputPortBase() {
	UnlinkAll();
}


void OutputPortBase::Link(InputPortBase* destination) {
	if (destination->link != nullptr) {
		throw InvalidArgumentException("Input port is already linked.");
	}

	if (destination->IsCompatible(GetType()) || GetType() == typeid(Any)) {
		links.insert(destination);
		destination->SetLinkState(this);
	}
	else {
		std::stringstream ss;
		ss << GetType().name() << " -> " << destination->GetType().name();
		throw InvalidArgumentException("Port types are not compatible.", ss.str());
	}
}


void OutputPortBase::Unlink(InputPortBase* other) {
	std::set<InputPortBase*>::iterator it;

	it = links.find(other);
	if (it != links.end()) {
		links.erase(it);
		other->SetLinkState(nullptr);
		return;
	}
}


void OutputPortBase::UnlinkAll() {
	for (auto v : links) {
		v->SetLinkState(nullptr);
	}
	links.clear();
}

OutputPortBase::LinkIterator OutputPortBase::begin() {
	return links.begin();
}
OutputPortBase::LinkIterator OutputPortBase::end() {
	return links.end();
}
OutputPortBase::ConstLinkIterator OutputPortBase::begin() const {
	return links.begin();
}
OutputPortBase::ConstLinkIterator OutputPortBase::end() const {
	return links.end();
}
OutputPortBase::ConstLinkIterator OutputPortBase::cbegin() const {
	return links.cbegin();
}
OutputPortBase::ConstLinkIterator OutputPortBase::cend() const {
	return links.cend();
}



// This single function had to be moved to this cpp file because
// the declaration of InputPort<AnyType>::Set(void) is not available
// if this function definition is inlined to the class.
void OutputPort<void>::Set() {
	for (auto v : links) {
		if (v->GetType() == typeid(void)) {
			static_cast<InputPort<void>*>(v)->Set();
		}
		else {
			assert(false);
		}
	}
}



// explicit instantiations
template class InputPort<Any>;
template class OutputPort<Any>;


} // namespace inl
