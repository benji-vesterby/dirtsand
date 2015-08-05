/******************************************************************************
 * This file is part of dirtsand.                                             *
 *                                                                            *
 * dirtsand is free software: you can redistribute it and/or modify           *
 * it under the terms of the GNU Affero General Public License as             *
 * published by the Free Software Foundation, either version 3 of the         *
 * License, or (at your option) any later version.                            *
 *                                                                            *
 * dirtsand is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU Affero General Public License for more details.                        *
 *                                                                            *
 * You should have received a copy of the GNU Affero General Public License   *
 * along with dirtsand.  If not, see <http://www.gnu.org/licenses/>.          *
 ******************************************************************************/

#ifndef _MOUL_PSEUDOLINKEFFECTMSG_H
#define _MOUL_PSEUDOLINKEFFECTMSG_H

#include "Message.h"

namespace MOUL
{
    class PseudoLinkEffectMsg : public Message
    {
        FACTORY_CREATABLE(PseudoLinkEffectMsg)

        virtual void read(DS::Stream* s);
        virtual void write(DS::Stream* s) const;

    public:
        Key m_linkObj, m_avatar;

    protected:
        PseudoLinkEffectMsg(uint16_t type) : Message(type) { }
    };
};

#endif // _MOUL_PSEUDOLINKEFFECTMSG_H