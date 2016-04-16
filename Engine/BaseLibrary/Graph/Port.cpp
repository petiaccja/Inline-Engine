#include "Port.hpp"
#include "Node.hpp"

namespace exc {


////////////////////////////////////////////////////////////////////////////////
// InputPortBase
//
////////////////////////////////////////////////////////////////////////////////


InputPortBase::InputPortBase() {
	link = nullptr;
}


InputPortBase::~InputPortBase() {
	Unlink();
}


bool InputPortBase::Link(OutputPortBase* source) {
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




////////////////////////////////////////////////////////////////////////////////
// OutputPortBase
//
////////////////////////////////////////////////////////////////////////////////

OutputPortBase::OutputPortBase() {
	// = default
}


OutputPortBase::~OutputPortBase() {
	UnlinkAll();
}


bool OutputPortBase::Link(InputPortBase* destination) {
	if (destination->link != nullptr) {
		return false;
	}

	// anytype ports receive special treatment
	if (destination->GetType() == typeid(AnyType)) {
		InputPort<AnyType>* destSpec = static_cast<InputPort<AnyType>*>(destination);

		// TYPE CONSTRAINT NOT IMPLEMENTED YET
		//if (destSpec->GetCurrentType() == this->GetType() || destSpec->GetCurrentType() == typeid(AnyType)) {
		//	// set link
		//	anyLinks.insert(destination);
		//	destination->SetLinkState(this);

		//	// create a type constraint
		//	// TODO...

		//	return true;
		//}
		//else {
		//	return false;
		//}

		anyLinks.insert(destination);
		destination->SetLinkState(this);
		return true;
	}
	else if (destination->GetType() == this->GetType()) {
		links.insert(destination);
		destination->SetLinkState(this);
		return true;
	}
	else if (this->GetType() == typeid(AnyType)) {
		links.insert(destination);
		destination->SetLinkState(this);
		return true;
	}
	else {
		return false;
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

	it = anyLinks.find(other);
	if (it != anyLinks.end()) {
		anyLinks.erase(it);
		other->SetLinkState(nullptr);
		return;
	}
}


void OutputPortBase::UnlinkAll() {
	for (auto v : links) {
		v->SetLinkState(nullptr);
	}
	for (auto v : anyLinks) {
		v->SetLinkState(nullptr);
	}
	links.clear();
	anyLinks.clear();
}

OutputPortBase::LinkIterator OutputPortBase::begin() {
	return LinkIterator(this, true);
}
OutputPortBase::LinkIterator OutputPortBase::end() {
	return LinkIterator(this, false);
}
OutputPortBase::ConstLinkIterator OutputPortBase::begin() const {
	return ConstLinkIterator(this, true);
}
OutputPortBase::ConstLinkIterator OutputPortBase::end() const {
	return ConstLinkIterator(this, false);
}
OutputPortBase::ConstLinkIterator OutputPortBase::cbegin() const {
	return ConstLinkIterator(this, true);
}
OutputPortBase::ConstLinkIterator OutputPortBase::cend() const {
	return ConstLinkIterator(this, false);
}



// This single function had to be moved to this cpp file because
// the declaration of InputPort<AnyType>::Set(void) is not available
// if this function definition is inlined to the class.
void OutputPort<void>::Set() {
	for (auto v : links) {
		static_cast<InputPort<void>*>(v)->Set();
	}
	for (auto v : anyLinks) {
		static_cast<InputPort<AnyType>*>(v)->Set();
	}
}



// explicit instantiations
template class InputPort<AnyType>;
template class OutputPort<AnyType>;


} // namespace exc