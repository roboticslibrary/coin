/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2001 by Systems in Motion. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  version 2.1 as published by the Free Software Foundation. See the
 *  file LICENSE.LGPL at the root directory of the distribution for
 *  more details.
 *
 *  If you want to use Coin for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition license.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

/*!
  \class SoActionMethodList SoActionMethodList.h Inventor/lists/SoActionMethodList.h
  \brief The SoActionMethodList class contains function pointers for action methods.
  \ingroup actions

  An SoActionMethodList contains one function pointer per node
  type. Each action contains an SoActioMethodList to know which
  functions to call during scene graph traversal.
*/

#include <Inventor/lists/SoActionMethodList.h>
#include <Inventor/lists/SoTypeList.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/actions/SoAction.h>
#include <Inventor/nodes/SoNode.h>
#include <assert.h>

#ifndef DOXYGEN_SKIP_THIS

class SoActionMethodListP {
public:
  SoActionMethodList * parent;
  int setupnumtypes;
  SbList <SoType> addedtypes;
  SbList <SoActionMethod> addedmethods;
};

#endif // DOXYGEN_SKIP_THIS

#undef THIS
#define THIS this->pimpl

/*!
  The constructor.  The \a parentlist argument is the parent action's
  action method list.  It can be \c NULL for action method lists that
  are not based on inheriting from a parent action.
*/
SoActionMethodList::SoActionMethodList(SoActionMethodList * const parentlist)
{
  THIS = new SoActionMethodListP;
  THIS->parent = parentlist;
  THIS->setupnumtypes = 0;
}

/*!
  Destructor.
*/
SoActionMethodList::~SoActionMethodList()
{
  delete THIS;
}

/*!
  Overloaded from parent to cast from \c void pointer.
*/
SoActionMethod &
SoActionMethodList::operator[](const int index)
{
  return (SoActionMethod&)SbPList::operator[](index);
}

/*!
  Add a function pointer to a node type's action method.
*/
void
SoActionMethodList::addMethod(const SoType node, const SoActionMethod method)
{
  THIS->addedtypes.append(node);
  THIS->addedmethods.append(method);
  THIS->setupnumtypes = 0; // force a new setUp
}

/*!
  This method must be called as the last initialization step before
  using the list. It fills in \c NULL entries with the parent's
  method.
*/
void
SoActionMethodList::setUp(void)
{
  if (THIS->setupnumtypes != SoType::getNumTypes()) {
    THIS->setupnumtypes = SoType::getNumTypes();
    this->truncate(0);

    SoTypeList derivedtypes;
    int i, n = THIS->addedtypes.getLength();
    for (i = 0; i < n; i++) {
      SoType type = THIS->addedtypes[i];
      const SoActionMethod method = THIS->addedmethods[i];
      (*this)[(int)type.getData()] = method;
      
      // also set this method for all nodes that inherits this node
      derivedtypes.truncate(0);
      int numderived = SoType::getAllDerivedFrom(THIS->addedtypes[i], derivedtypes);
      for (int j = 0; j < numderived; j++) {
        int idx = (int) derivedtypes[j].getData();
        (*this)[idx] = method;
      }
    }
    
    // fill in nullAction for all nodetypes with method == NULL
    derivedtypes.truncate(0);
    (void) SoType::getAllDerivedFrom(SoNode::getClassTypeId(), derivedtypes);
    
    for (i = 0; i < this->getLength(); i++) {
      if ((*this)[i] == NULL) (*this)[i] = SoAction::nullAction;
    }
    for (i = this->getLength(); i < derivedtypes.getLength(); i++) {
      this->append((void*) SoAction::nullAction);
    }
    
    // fill in empty slots with parent method
    if (THIS->parent) {
      THIS->parent->setUp();
      n = THIS->parent->getLength();
      for (i = 0; i < n; i++) {
        if ((*this)[i] == SoAction::nullAction) (*this)[i] = (*(THIS->parent))[i];
      }
    }
  }
}

#undef THIS
