/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*

An object archiving system for SuperCollider.

*/


#ifndef _PyrDeepCopier_
#define _PyrDeepCopier_

#include "PyrObject.h"
#include "SC_AllocPool.h"

#include "PyrKernel.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"
#include "GC.h"
#include "ReadWriteMacros.h"

const int32 kDeepCopierObjectArrayInitialCapacity = 32;

class PyrDeepCopier
{
public:
	PyrDeepCopier(VMGlobals *inG)
		: g(inG), objectArray(initialObjectArray), numObjects(0), 
			objectArrayCapacity( kDeepCopierObjectArrayInitialCapacity )
		{
		}
	
	~PyrDeepCopier()
		{
			if (objectArrayCapacity > kDeepCopierObjectArrayInitialCapacity) {
				g->allocPool->Free(objectArray);
			}
		}
			
	long doDeepCopy(PyrSlot *objectSlot)
		{
			long err = errNone;
				
			try {
				if (IsObj(objectSlot)) {
					constructObjectArray(objectSlot->uo);				
					for (int i=0; i<numObjects; ++i) {
						fixSlots(objectArray[i]);
					}
					fixObjSlot(objectSlot);
				}
			} catch (std::exception &ex) {
				error(ex.what());
				err = errFailed;
			}
			return err;
		}
	
private:
			
	void recurse(PyrObject *obj, int n)
		{	
			//post("recurse %s %08X\n", obj->classptr->name.us->name, obj);
			PyrSlot *slot = obj->slots;
			for (int i=0; i<n; ++i, ++slot) {
				if (IsObj(slot)) constructObjectArray(slot->uo);
			}
		}

	void growObjectArray()
		{
			int32 newObjectArrayCapacity = objectArrayCapacity << 1;
			
			int32 newSize = newObjectArrayCapacity * sizeof(PyrObject*);
			PyrObject** newArray = (PyrObject**)g->allocPool->Alloc(newSize);
			memcpy(newArray, objectArray, numObjects * sizeof(PyrObject*));
			if (objectArrayCapacity > kDeepCopierObjectArrayInitialCapacity) {
				g->allocPool->Free(objectArray);
			}
			objectArrayCapacity = newObjectArrayCapacity;
			objectArray = newArray;
		}
		
	void putSelf(PyrObject *obj)
		{
			obj->SetMark();
			obj->scratch1 = numObjects;
			
			// expand array if needed
			if (numObjects >= objectArrayCapacity) growObjectArray();
			
			// add to array
			objectArray[numObjects++] = obj;
		}

	void putCopy(PyrObject *obj)
		{
			obj->SetMark();
			obj->scratch1 = numObjects;
			
			// expand array if needed
			if (numObjects >= objectArrayCapacity) growObjectArray();
			
			// add a shallow copy to object array
			PyrObject *copy = copyObject(g->gc, obj, false);
			copy->ClearMark();
			
			objectArray[numObjects++] = copy;
		}

	void constructObjectArray(PyrObject *obj)
		{
			//post("constructObjectArray %s %08X\n", obj->classptr->name.us->name, obj);
			if (!obj->IsMarked()) {
				if (isKindOf(obj, class_class)) {
					putSelf(obj);
				} else if (isKindOf(obj, class_process)) {
					putSelf(obj);
				} else if (isKindOf(obj, s_interpreter->u.classobj)) {
					putSelf(obj);
				} else if (isKindOf(obj, class_rawarray)) {
					putCopy(obj);
				} else if (isKindOf(obj, class_array)) {
					putCopy(obj);
					recurse(obj, obj->size);
				} else if (isKindOf(obj, class_func)) {
					putSelf(obj);
				} else if (isKindOf(obj, class_method)) {
					putSelf(obj);
				} else if (isKindOf(obj, class_thread)) {
					putSelf(obj);
				} else {
					putCopy(obj);
					recurse(obj, obj->size);
				}
			}
		}

	void fixObjSlot(PyrSlot* slot) 
		{
			//post("fixObjSlot %s %08X %08X\n", slot->uo->classptr->name.us->name, slot->uo, objectArray[slot->uo->scratch1]);
			slot->uo = objectArray[slot->uo->scratch1];
		}
	
	void fixSlots(PyrObject *obj)
		{
			//post("fixSlots %s %08X %d\n", obj->classptr->name.us->name, obj, obj->IsMarked());
			if (!obj->IsMarked() && obj->obj_format <= obj_slot) { // it is a copy
				PyrSlot *slot = obj->slots;
				for (int i=0; i<obj->size; ++i, ++slot) {
					if (IsObj(slot)) fixObjSlot(slot);
				}
			}
		}
		
	VMGlobals *g;
		
	PyrObject **objectArray;
	int32 numObjects;
	int32 objectArrayCapacity;
	
	PyrObject *initialObjectArray[kDeepCopierObjectArrayInitialCapacity];
};

/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*

An object archiving system for SuperCollider.

*/



#endif

