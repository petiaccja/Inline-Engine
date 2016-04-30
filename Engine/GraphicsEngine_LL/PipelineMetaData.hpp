#pragma once

// this is supposed to be like std::optional
// if underlying graph changes, no data
template <class T> 
class PipelineMetaData {
public:

	bool IsFresh() {
		
	}
	template <class... Args>
	void Initialize();
private:
	bool m_fresh;
	T data;
};