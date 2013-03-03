/*
 * AttributesOverlay.h
 *
 *  Created on: Feb 16, 2013
 *      Author: Guillaume Chatelet
 */

#ifndef ATTRIBUTESOVERLAY_H_
#define ATTRIBUTESOVERLAY_H_

#include "IOverlay.h"
#include <memory>

namespace duke {

struct Context;
struct GlyphRenderer;

class AttributesOverlay: public IOverlay {
public:
	AttributesOverlay(const GlyphRenderer&);
	virtual void render(const Context&) const;
private:
	const GlyphRenderer &m_GlyphRenderer;
};

} /* namespace duke */
#endif /* ATTRIBUTESOVERLAY_H_ */
