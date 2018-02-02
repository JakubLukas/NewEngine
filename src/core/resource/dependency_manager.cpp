#include "dependency_manager.h"


namespace Veng
{


void DependencyManager::ResourceLoaded(ResourceType resourceType, resourceHandle handle)
{
	Array<DependencyTmp> loaded(m_allocator);

	for (size_t i = 0; i < m_dependenciesTmp.GetSize();)
	{
		DependencyTmp& tmp = m_dependenciesTmp[i];
		if (tmp.childType == resourceType && tmp.childHandle == handle)
		{
			loaded.Push(tmp);
			//tmp.parent->ChildResourceLoaded(handle);
			m_dependenciesTmp.Erase((unsigned)i);
		}
		else
		{
			i++;
		}
	}

	for (DependencyTmp& tmp : loaded)
	{
		tmp.parent->ChildResourceLoaded(handle);
	}
}


}