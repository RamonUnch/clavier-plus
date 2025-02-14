// Clavier+
// Keyboard shortcuts manager
//
// Copyright (C) 2000-2008 Guillaume Ryder
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "StdAfx.h"
#include "../Com.h"

#include <msi.h>

namespace ComTest {

TEST_CLASS(CoPtrTest) {
public:
	
	TEST_METHOD(constructorEmpty) {
		CoPtr<IShellLink> ptr;
		Assert::IsFalse(ptr);
	}
	
	TEST_METHOD(constructorCreateInstance) {
		CoPtr<IShellLink> ptr(CLSID_ShellLink);
		Assert::IsTrue(ptr);
		assertOneReference(ptr);
	}
	
	TEST_METHOD(constructorMove) {
		CoPtr<IShellLink> ptr1(CLSID_ShellLink);
		IShellLink* raw_ptr = ptr1.get();
		
		CoPtr<IShellLink> ptr2 = std::move(ptr1);
		
		Assert::IsFalse(ptr1);
		Assert::IsTrue(ptr2);
		Assert::AreEqual(raw_ptr, ptr2.get());
		assertOneReference(ptr2);
	}
	
	TEST_METHOD(assignmentMove_validToNull) {
		CoPtr<IShellLink> ptr1(CLSID_ShellLink);
		IShellLink* raw_ptr = ptr1.get();
		
		CoPtr<IShellLink> ptr2;
		ptr2 = std::move(ptr1);
		
		Assert::IsFalse(ptr1);
		Assert::IsTrue(ptr2);
		Assert::AreEqual(raw_ptr, ptr2.get());
		assertOneReference(ptr2);
	}
	
	TEST_METHOD(assignmentMove_nullToValid) {
		CoPtr<IShellLink> ptr1;
		
		CoPtr<IShellLink> ptr2(CLSID_ShellLink);
		IShellLink* raw_ptr = ptr2.get();
		raw_ptr->AddRef();
		
		ptr2 = std::move(ptr1);
		
		Assert::IsFalse(ptr1);
		Assert::IsFalse(ptr2);
		Assert::AreEqual(0LU, raw_ptr->Release());
	}
	
	TEST_METHOD(assignmentMove_validToSelf) {
		CoPtr<IShellLink> ptr(CLSID_ShellLink);
		IShellLink* raw_ptr = ptr.get();
		
		ptr = std::move(ptr);
		
		Assert::AreEqual(raw_ptr, ptr.get());
		assertOneReference(ptr);
	}
	
	TEST_METHOD(assignmentMove_nullToNull) {
		CoPtr<IShellLink> ptr1;
		
		CoPtr<IShellLink> ptr2;
		ptr2 = std::move(ptr1);
		
		Assert::IsFalse(ptr1);
		Assert::IsFalse(ptr2);
	}
	
	TEST_METHOD(destructor_releases) {
		IShellLink* raw_ptr;
		{
			CoPtr<IShellLink> ptr(CLSID_ShellLink);
			raw_ptr = ptr.get();
			Assert::AreEqual(2LU, raw_ptr->AddRef());
		}
		Assert::AreEqual(0LU, raw_ptr->Release());
	}
	
	TEST_METHOD(arrowOperator_nullFails) {
		CoPtr<IShellLink> ptr;
		Assert::ExpectException<std::runtime_error>([&] {
			ptr->AddRef();
		});
	}
	
	TEST_METHOD(get_validPtr) {
		CoPtr<IShellLink> ptr(CLSID_ShellLink);
		IShellLink* raw_ptr = ptr.get();
		Assert::IsNotNull(raw_ptr);
		assertOneReference(raw_ptr);
	}
	
	TEST_METHOD(get_nullFails) {
		CoPtr<IShellLink> ptr;
		Assert::ExpectException<std::runtime_error>([&] {
			ptr.get();
		});
	}
	
	TEST_METHOD(queryInterface_validPtr) {
		CoPtr<IShellLink> ptr_shell_link(CLSID_ShellLink);
		CoPtr<IUnknown> ptr_unknown = ptr_shell_link.queryInterface<IUnknown>();
		Assert::AreNotEqual(static_cast<void*>(ptr_shell_link.get()), static_cast<void*>(ptr_unknown.get()));
	}
	
	TEST_METHOD(queryInterface_nullFails) {
		CoPtr<IShellLink> ptr;
		Assert::ExpectException<std::runtime_error>([&] {
			ptr.queryInterface<IUnknown>();
		});
	}
	
	TEST_METHOD(addressOperator_nullSucceeds) {
		IShellLink* raw_ptr;
		{
			CoPtr<IShellLink> ptr;
			IShellLink** address = &ptr;
			Assert::IsNotNull(address);
			Assert::IsNull(*address);
			
			Assert::IsTrue(SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(address))));
			raw_ptr = *address;
			
			Assert::AreEqual(raw_ptr, ptr.get());
			raw_ptr->AddRef();
		}
		assertOneReference(raw_ptr);
	}
	
	TEST_METHOD(addressOperator_validPtrFails) {
		CoPtr<IShellLink> ptr(CLSID_ShellLink);
		Assert::ExpectException<std::runtime_error>([&] {
			&ptr;
		});
	}
	
private:
	
	void assertOneReference(IUnknown* object) {
		Assert::AreEqual(2UL, object->AddRef());
		Assert::AreEqual(1UL, object->Release());
	}
	
	template<class T>
	void assertOneReference(CoPtr<T>& ptr) {
		Assert::AreEqual(2UL, ptr->AddRef());
		Assert::AreEqual(1UL, ptr->Release());
	}
};

TEST_CLASS(CoBufferTest) {
public:
	
	TEST_METHOD(constructorEmpty) {
		CoBuffer<char*> buffer;
		Assert::IsNull(static_cast<const char*>(buffer));
	}
	
	TEST_METHOD(constructorTakeOwnership) {
		char* raw_buffer = static_cast<char*>(CoTaskMemAlloc(10));
		lstrcpyA(raw_buffer, "test");
		
		CoBuffer<char*> buffer(raw_buffer);
		Assert::IsNotNull(static_cast<char*>(buffer));
		Assert::AreSame(*raw_buffer, *static_cast<char*>(buffer));
	}
	
	TEST_METHOD(destructor_freesValid) {
		char* raw_buffer = static_cast<char*>(CoTaskMemAlloc(10));
		
		CoPtr<IMalloc> malloc;
		Assert::IsTrue(SUCCEEDED(CoGetMalloc(1, &malloc)));
		Assert::AreEqual(1, malloc->DidAlloc(raw_buffer));
		
		{
			CoBuffer<char*> buffer(raw_buffer);
		}
		
		Assert::AreEqual(0, malloc->DidAlloc(raw_buffer));
	}
	
	TEST_METHOD(addressOperator_nullSucceeds) {
		char* raw_buffer = static_cast<char*>(CoTaskMemAlloc(10));
		
		CoBuffer<char*> buffer;
		char **address = &buffer;
		Assert::IsNotNull(address);
		Assert::IsNull(*address);
		
		*address = raw_buffer;
		Assert::AreEqual(raw_buffer, buffer);
	}
	
	TEST_METHOD(addressOperator_validPtrFails) {
		CoBuffer<char*> buffer(static_cast<char*>(CoTaskMemAlloc(10)));
		Assert::ExpectException<std::runtime_error>([&] {
			&buffer;
		});
	}
};

}  // namespace ComTest
