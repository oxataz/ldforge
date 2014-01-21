/*
 *  LDForge: LDraw parts authoring CAD
 *  Copyright (C) 2013, 2014 Santeri Piippo
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LDFORGE_LDTYPES_H
#define LDFORGE_LDTYPES_H

#include "Main.h"
#include "Types.h"
#include "misc/DocumentPointer.h"

#define LDOBJ(T)										\
protected:												\
	virtual LD##T* clone() override						\
	{													\
		return new LD##T (*this);						\
	}													\
														\
public:													\
	virtual LDObject::Type getType() const override		\
	{													\
		return LDObject::E##T;							\
	}													\
	virtual QString raw() const override;				\
	virtual void invert() override;

#define LDOBJ_NAME(N)          virtual QString getTypeName() const override { return #N; }
#define LDOBJ_VERTICES(V)      virtual int vertices() const override { return V; }
#define LDOBJ_SETCOLORED(V)    virtual bool isColored() const override { return V; }
#define LDOBJ_COLORED          LDOBJ_SETCOLORED (true)
#define LDOBJ_UNCOLORED        LDOBJ_SETCOLORED (false)

#define LDOBJ_CUSTOM_SCEMANTIC virtual bool isScemantic() const override
#define LDOBJ_SCEMANTIC        LDOBJ_CUSTOM_SCEMANTIC { return true; }
#define LDOBJ_NON_SCEMANTIC    LDOBJ_CUSTOM_SCEMANTIC { return false; }

#define LDOBJ_SETMATRIX(V)     virtual bool hasMatrix() const override { return V; }
#define LDOBJ_HAS_MATRIX       LDOBJ_SETMATRIX (true)
#define LDOBJ_NO_MATRIX        LDOBJ_SETMATRIX (false)

class QListWidgetItem;
class LDSubfile;
class LDDocument;
class LDSharedVertex;

// =============================================================================
// LDObject
//
// Base class object for all object types. Each LDObject represents a single line
// in the LDraw code file. The virtual method getType returns an enumerator
// which is a token of the object's type. The object can be casted into
// sub-classes based on this enumerator.
// =============================================================================
class LDObject
{
	PROPERTY (public,		bool,			Hidden,			BOOL_OPS,	STOCK_WRITE)
	PROPERTY (public,		bool,			Selected,		BOOL_OPS,	STOCK_WRITE)
	PROPERTY (public,		LDObject*,		Parent,			NO_OPS,		STOCK_WRITE)
	PROPERTY (public,		LDDocument*,	File,			NO_OPS,		STOCK_WRITE) // TODO: rename~
	PROPERTY (private,		int,			ID,				NUM_OPS,	STOCK_WRITE)
	PROPERTY (public,		int,			Color,			NUM_OPS,	CUSTOM_WRITE)
	PROPERTY (public,		bool,			GLInit,			BOOL_OPS,	STOCK_WRITE)

	public:
		// Object type codes.
		enum Type
		{
			ESubfile,        // Object represents a sub-file reference
			EQuad,           // Object represents a quadrilateral
			ETriangle,       // Object represents a triangle
			ELine,           // Object represents a line
			ECondLine,       // Object represents a conditional line
			EVertex,         // Object is a vertex, LDForge extension object
			EBFC,            // Object represents a BFC statement
			EOverlay,        // Object contains meta-info about an overlay image.
			EComment,        // Object represents a comment
			EError,          // Object is the result of failed parsing
			EEmpty,          // Object represents an empty line
			EUnidentified,   // Unknown object type (some functions return this; TODO: they probably should not)
			ENumTypes        // Amount of object types
		};

		LDObject();

		// Makes a copy of this object
		LDObject*					createCopy() const;

		// Deletes this object
		void						deleteSelf();

		// Index (i.e. line number) of this object
		long						getIndex() const;

		// Type enumerator of this object
		virtual Type				getType() const = 0;

		// Get a vertex by index
		const Vertex&				getVertex (int i) const;

		// Type name of this object
		virtual QString				getTypeName() const = 0;

		// Does this object have a matrix and position? (see LDMatrixObject)
		virtual bool				hasMatrix() const = 0;

		// Inverts this object (winding is reversed)
		virtual void				invert() = 0;

		// Is this object colored?
		virtual bool				isColored() const = 0;

		// Does this object have meaning in the part model?
		virtual bool				isScemantic() const = 0;

		// Moves this object using the given vertex as a movement List
		void						move (Vertex vect);

		// Object after this in the current file
		LDObject*					next() const;

		// Object prior to this in the current file
		LDObject*					prev() const;

		// This object as LDraw code
		virtual						QString raw() const = 0;

		// Replace this LDObject with another LDObject. Object is deleted in the process.
		void						replace (LDObject* other);

		// Selects this object.
		void						select();

		// Set a vertex to the given value
		void						setVertex (int i, const Vertex& vert);

		// Set a single coordinate of a vertex
		void						setVertexCoord (int i, Axis ax, double value);

		// Swap this object with another.
		void						swap (LDObject* other);

		// What object in the current file ultimately references this?
		LDObject*					topLevelParent();

		// Removes this object from selection // TODO: rename to deselect?
		void						unselect();

		// Number of vertices this object has // TODO: rename to getNumVertices
		virtual int					vertices() const = 0;

		// Get type name by enumerator
		static QString typeName (LDObject::Type type);

		// Returns a default-constructed LDObject by the given type
		static LDObject* getDefault (const LDObject::Type type);

		// TODO: move this to LDDocument?
		static void moveObjects (LDObjectList objs, const bool up);

		// Get a description of a list of LDObjects
		static QString describeObjects (const LDObjectList& objs);
		static LDObject* fromID (int id);

		// TODO: make these private!
		// OpenGL list for this object
		uint glLists[4];

		// Object list entry for this object
		QListWidgetItem* qObjListEntry;

	protected:
		// LDObjects are to be deleted with the deleteSelf() method, not with
		// operator delete. This is because it seems virtual functions cannot
		// be properly called from the destructor, thus a normal method must
		// be used instead. The destructor also doesn't seem to be able to
		// be private without causing a truckload of problems so it's protected
		// instead.
		virtual ~LDObject();
		void chooseID();

	private:
		virtual LDObject* clone() = 0;
		LDSharedVertex*	m_coords[4];
};

// =============================================================================
// LDSharedVertex
//
// For use as coordinates of LDObjects. Keeps count of references.
// -----------------------------------------------------------------------------
class LDSharedVertex
{
	public:
		inline const Vertex& data() const
		{
			return m_data;
		}

		inline operator const Vertex&() const
		{
			return m_data;
		}

		void addRef (LDObject* a);
		void delRef (LDObject* a);

		static LDSharedVertex* getSharedVertex (const Vertex& a);

	protected:
		LDSharedVertex (const Vertex& a) : m_data (a) {}

	private:
		LDObjectList m_refs;
		Vertex m_data;
};

// =============================================================================
// LDMatrixObject
// =============================================================================
//
// Common code for objects with matrices. This class is multiple-derived in
// and thus not used directly other than as a common storage point for matrices
// and vertices.
//
// The link pointer is a pointer to this object's LDObject self - since this is
// multiple-derived in, static_cast or dynamic_cast won't budge here.
//
// In 0.1-alpha, there was a separate 'radial' type which had a position and
// matrix as well. Even though right now only LDSubfile uses this, I'm keeping
// this class distinct in case I get new extension ideas. :)
// =============================================================================
class LDMatrixObject
{
	PROPERTY (public,	LDObject*,			LinkPointer,	NO_OPS,	STOCK_WRITE)
	PROPERTY (public,	Matrix,				Transform,		NO_OPS,	CUSTOM_WRITE)

	public:
		LDMatrixObject() :
			m_Position (LDSharedVertex::getSharedVertex (g_origin)) {}

		LDMatrixObject (const Matrix& transform, const Vertex& pos) :
			m_Transform (transform),
			m_Position (LDSharedVertex::getSharedVertex (pos)) {}

		inline const Vertex& getPosition() const
		{
			return m_Position->data();
		}

		void setCoordinate (const Axis ax, double value)
		{
			Vertex v = getPosition();
			v[ax] = value;
			setPosition (v);
		}

		void setPosition (const Vertex& a);

	private:
		LDSharedVertex*	m_Position;
};

// =============================================================================
// LDError
//
// Represents a line in the LDraw file that could not be properly parsed. It is
// represented by a (!) ERROR in the code view. It exists for the purpose of
// allowing garbage lines be debugged and corrected within LDForge. The member
// zContent contains the contents of the unparsable line.
// =============================================================================
class LDError : public LDObject
{
	LDOBJ (Error)
	LDOBJ_NAME (error)
	LDOBJ_VERTICES (0)
	LDOBJ_UNCOLORED
	LDOBJ_SCEMANTIC
	LDOBJ_NO_MATRIX
	PROPERTY (public,	QString, FileReferenced, STR_OPS,	STOCK_WRITE)

	public:
		LDError();
		LDError (QString contents, QString reason) : contents (contents), reason (reason) {}

		// Content of this unknown line
		QString contents;

		// Why is this gibberish?
		QString reason;
};

// =============================================================================
// LDEmpty
//
// Represents an empty line in the LDraw code file.
// =============================================================================
class LDEmpty : public LDObject
{
	LDOBJ (Empty)
	LDOBJ_NAME (empty)
	LDOBJ_VERTICES (0)
	LDOBJ_UNCOLORED
	LDOBJ_NON_SCEMANTIC
	LDOBJ_NO_MATRIX
};

// =============================================================================
// LDComment
//
// Represents a code-0 comment in the LDraw code file. Member text contains
// the text of the comment.
// =============================================================================
class LDComment : public LDObject
{
	LDOBJ (Comment)
	LDOBJ_NAME (comment)
	LDOBJ_VERTICES (0)
	LDOBJ_UNCOLORED
	LDOBJ_NON_SCEMANTIC
	LDOBJ_NO_MATRIX

	public:
		LDComment() {}
		LDComment (QString text) : text (text) {}

		QString text; // The text of this comment
};

// =============================================================================
// LDBFC
//
// Represents a 0 BFC statement in the LDraw code. eStatement contains the type
// of this statement.
// =============================================================================
class LDBFC : public LDObject
{
	public:
		enum Type
		{
			CertifyCCW,
			CCW,
			CertifyCW,
			CW,
			NoCertify,
			InvertNext,
			Clip,
			ClipCCW,
			ClipCW,
			NoClip,
			NumStatements
		};

		LDOBJ (BFC)
		LDOBJ_NAME (bfc)
		LDOBJ_VERTICES (0)
		LDOBJ_UNCOLORED
		LDOBJ_CUSTOM_SCEMANTIC { return (type == InvertNext); }
		LDOBJ_NO_MATRIX

	public:
		LDBFC() {}
		LDBFC (const LDBFC::Type type) :
			type (type) {}

		// Statement strings
		static const char* statements[];

		Type type;
};

// =============================================================================
// LDSubfile
//
// Represents a single code-1 subfile reference.
// =============================================================================
class LDSubfile : public LDObject, public LDMatrixObject
{
	LDOBJ (Subfile)
	LDOBJ_NAME (subfile)
	LDOBJ_VERTICES (0)
	LDOBJ_COLORED
	LDOBJ_SCEMANTIC
	LDOBJ_HAS_MATRIX
	PROPERTY (public,	LDDocumentPointer, FileInfo, NO_OPS,	STOCK_WRITE)

	public:
		enum InlineFlag
		{
			DeepInline     = (1 << 0),
			CacheInline    = (1 << 1),
			RendererInline = (1 << 2),

			DeepCacheInline = DeepInline | CacheInline,
		};

		Q_DECLARE_FLAGS (InlineFlags, InlineFlag)

		LDSubfile()
		{
			setLinkPointer (this);
		}

		// Inlines this subfile. Note that return type is an array of heap-allocated
		// LDObject copies, they must be deleted manually.
		LDObjectList inlineContents (InlineFlags flags);

	protected:
		~LDSubfile();
};

Q_DECLARE_OPERATORS_FOR_FLAGS (LDSubfile::InlineFlags)

// =============================================================================
// LDLine
//
// Represents a single code-2 line in the LDraw code file. v0 and v1 are the end
// points of the line. The line is colored with dColor unless uncolored mode is
// set.
// =============================================================================
class LDLine : public LDObject
{
	LDOBJ (Line)
	LDOBJ_NAME (line)
	LDOBJ_VERTICES (2)
	LDOBJ_COLORED
	LDOBJ_SCEMANTIC
	LDOBJ_NO_MATRIX

	public:
		LDLine() {}
		LDLine (Vertex v1, Vertex v2);
};

// =============================================================================
// LDCondLine
//
// Represents a single code-5 conditional line. The end-points v0 and v1 are
// inherited from LDLine, c0 and c1 are the control points of this line.
// =============================================================================
class LDCondLine : public LDLine
{
	LDOBJ (CondLine)
	LDOBJ_NAME (condline)
	LDOBJ_VERTICES (4)
	LDOBJ_COLORED
	LDOBJ_SCEMANTIC
	LDOBJ_NO_MATRIX

	public:
		LDCondLine() {}
		LDLine* demote();
};

// =============================================================================
// LDTriangle
//
// Represents a single code-3 triangle in the LDraw code file. Vertices v0, v1
// and v2 contain the end-points of this triangle. dColor is the color the
// triangle is colored with.
// =============================================================================
class LDTriangle : public LDObject
{
	LDOBJ (Triangle)
	LDOBJ_NAME (triangle)
	LDOBJ_VERTICES (3)
	LDOBJ_COLORED
	LDOBJ_SCEMANTIC
	LDOBJ_NO_MATRIX

	public:
		LDTriangle() {}
		LDTriangle (Vertex v0, Vertex v1, Vertex v2)
		{
			setVertex (0, v0);
			setVertex (1, v1);
			setVertex (2, v2);
		}
};

// =============================================================================
// LDQuad
//
// Represents a single code-4 quadrilateral. v0, v1, v2 and v3 are the end points
// of the quad, dColor is the color used for the quad.
// =============================================================================
class LDQuad : public LDObject
{
	LDOBJ (Quad)
	LDOBJ_NAME (quad)
	LDOBJ_VERTICES (4)
	LDOBJ_COLORED
	LDOBJ_SCEMANTIC
	LDOBJ_NO_MATRIX

	public:
		LDQuad() {}
		LDQuad (const Vertex& v0, const Vertex& v1, const Vertex& v2, const Vertex& v3);

		// Split this quad into two triangles (note: heap-allocated)
		QList<LDTriangle*> splitToTriangles();
};

// =============================================================================
// LDVertex
//
// The vertex is an LDForce-specific extension which represents a single
// vertex which can be used as a parameter to tools or to store coordinates
// with. Vertices are a part authoring tool and they should not appear in
// finished parts.
// =============================================================================
class LDVertex : public LDObject
{
	LDOBJ (Vertex)
	LDOBJ_NAME (vertex)
	LDOBJ_VERTICES (0) // TODO: move pos to vaCoords[0]
	LDOBJ_COLORED
	LDOBJ_NON_SCEMANTIC
	LDOBJ_NO_MATRIX

	public:
		LDVertex() {}

		Vertex pos;
};

// =============================================================================
// LDOverlay
//
// Overlay image meta, stored in the header of parts so as to preserve overlay
// information.
// =============================================================================
class LDOverlay : public LDObject
{
	LDOBJ (Overlay)
	LDOBJ_NAME (overlay)
	LDOBJ_VERTICES (0)
	LDOBJ_UNCOLORED
	LDOBJ_NON_SCEMANTIC
	LDOBJ_NO_MATRIX
	PROPERTY (public,	int,	 Camera,	NUM_OPS,	STOCK_WRITE)
	PROPERTY (public,	int,	 X,			NUM_OPS,	STOCK_WRITE)
	PROPERTY (public,	int,	 Y,			NUM_OPS,	STOCK_WRITE)
	PROPERTY (public,	int,	 Width,		NUM_OPS,	STOCK_WRITE)
	PROPERTY (public,	int,	 Height,	NUM_OPS,	STOCK_WRITE)
	PROPERTY (public,	QString, FileName,	STR_OPS,	STOCK_WRITE)
};

// Other common LDraw stuff
static const QString CALicense = "!LICENSE Redistributable under CCAL version 2.0 : see CAreadme.txt",
				 NonCALicense = "!LICENSE Not redistributable : see NonCAreadme.txt";
static const int lores = 16;
static const int hires = 48;

QString getLicenseText (int id);

#endif // LDFORGE_LDTYPES_H