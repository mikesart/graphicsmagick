/*
 * valid.h : interface to the DTD handling and the validity checking
 *
 * See Copyright for the status of this software.
 *
 * Daniel.Veillard@w3.org
 */


#ifndef __XML_VALID_H__
#define __XML_VALID_H__

#include <libxml/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * an xmlValidCtxt is used for error reporting when validating
 */

typedef void (*xmlValidityErrorFunc) (void *ctx, const char *msg, ...);
typedef void (*xmlValidityWarningFunc) (void *ctx, const char *msg, ...);

typedef struct _xmlValidCtxt xmlValidCtxt;
typedef xmlValidCtxt *xmlValidCtxtPtr;
struct _xmlValidCtxt {
    void *userData;			/* user specific data block */
    xmlValidityErrorFunc error;		/* the callback in case of errors */
    xmlValidityWarningFunc warning;	/* the callback in case of warning */

    /* Node analysis stack used when validating within entities */
    xmlNodePtr         node;          /* Current parsed Node */
    int                nodeNr;        /* Depth of the parsing stack */
    int                nodeMax;       /* Max depth of the parsing stack */
    xmlNodePtr        *nodeTab;       /* array of nodes */

    int              finishDtd;       /* finished validating the Dtd ? */
    xmlDocPtr              doc;       /* the document */
    int                  valid;       /* temporary validity check result */
};

/*
 * ALl notation declarations are stored in a table
 * there is one table per DTD
 */

typedef struct _xmlHashTable xmlNotationTable;
typedef xmlNotationTable *xmlNotationTablePtr;

/*
 * ALl element declarations are stored in a table
 * there is one table per DTD
 */

typedef struct _xmlHashTable xmlElementTable;
typedef xmlElementTable *xmlElementTablePtr;

/*
 * ALl attribute declarations are stored in a table
 * there is one table per DTD
 */

typedef struct _xmlHashTable xmlAttributeTable;
typedef xmlAttributeTable *xmlAttributeTablePtr;

/*
 * ALl IDs attributes are stored in a table
 * there is one table per document
 */

typedef struct _xmlHashTable xmlIDTable;
typedef xmlIDTable *xmlIDTablePtr;

/*
 * ALl Refs attributes are stored in a table
 * there is one table per document
 */

typedef struct _xmlHashTable xmlRefTable;
typedef xmlRefTable *xmlRefTablePtr;

/* helper */
xmlChar *           xmlSplitQName2	(const xmlChar *name,
					 xmlChar **prefix);

/* Notation */
xmlNotationPtr	    xmlAddNotationDecl	(xmlValidCtxtPtr ctxt,
					 xmlDtdPtr dtd,
					 const xmlChar *name,
					 const xmlChar *PublicID,
					 const xmlChar *SystemID);
xmlNotationTablePtr xmlCopyNotationTable(xmlNotationTablePtr table);
void		    xmlFreeNotationTable(xmlNotationTablePtr table);
void		    xmlDumpNotationDecl	(xmlBufferPtr buf,
					 xmlNotationPtr nota);
void		    xmlDumpNotationTable(xmlBufferPtr buf,
					 xmlNotationTablePtr table);

/* Element Content */
xmlElementContentPtr xmlNewElementContent (xmlChar *name,
					   xmlElementContentType type);
xmlElementContentPtr xmlCopyElementContent(xmlElementContentPtr content);
void		     xmlFreeElementContent(xmlElementContentPtr cur);
void		     xmlSprintfElementContent(char *buf,
	                                   xmlElementContentPtr content,
					   int glob);

/* Element */
xmlElementPtr	   xmlAddElementDecl	(xmlValidCtxtPtr ctxt,
					 xmlDtdPtr dtd,
					 const xmlChar *name,
					 xmlElementTypeVal type,
					 xmlElementContentPtr content);
xmlElementTablePtr xmlCopyElementTable	(xmlElementTablePtr table);
void		   xmlFreeElementTable	(xmlElementTablePtr table);
void		   xmlDumpElementTable	(xmlBufferPtr buf,
					 xmlElementTablePtr table);
void		   xmlDumpElementDecl	(xmlBufferPtr buf,
					 xmlElementPtr elem);

/* Enumeration */
xmlEnumerationPtr  xmlCreateEnumeration	(xmlChar *name);
void		   xmlFreeEnumeration	(xmlEnumerationPtr cur);
xmlEnumerationPtr  xmlCopyEnumeration	(xmlEnumerationPtr cur);

/* Attribute */
xmlAttributePtr	    xmlAddAttributeDecl	    (xmlValidCtxtPtr ctxt,
					     xmlDtdPtr dtd,
					     const xmlChar *elem,
					     const xmlChar *name,
					     const xmlChar *ns,
					     xmlAttributeType type,
					     xmlAttributeDefault def,
					     const xmlChar *defaultValue,
					     xmlEnumerationPtr tree);
xmlAttributeTablePtr xmlCopyAttributeTable  (xmlAttributeTablePtr table);
void		     xmlFreeAttributeTable  (xmlAttributeTablePtr table);
void		     xmlDumpAttributeTable  (xmlBufferPtr buf,
					     xmlAttributeTablePtr table);
void		     xmlDumpAttributeDecl   (xmlBufferPtr buf,
					     xmlAttributePtr attr);

/* IDs */
xmlIDPtr	xmlAddID	(xmlValidCtxtPtr ctxt,
				 xmlDocPtr doc,
				 const xmlChar *value,
				 xmlAttrPtr attr);
xmlIDTablePtr	xmlCopyIDTable	(xmlIDTablePtr table);
void		xmlFreeIDTable	(xmlIDTablePtr table);
xmlAttrPtr	xmlGetID	(xmlDocPtr doc,
				 const xmlChar *ID);
int		xmlIsID		(xmlDocPtr doc,
				 xmlNodePtr elem,
				 xmlAttrPtr attr);
int		xmlRemoveID	(xmlDocPtr doc, xmlAttrPtr attr);

/* IDREFs */
xmlRefPtr	xmlAddRef	(xmlValidCtxtPtr ctxt,
				 xmlDocPtr doc,
				 const xmlChar *value,
				 xmlAttrPtr attr);
xmlRefTablePtr	xmlCopyRefTable	(xmlRefTablePtr table);
void		xmlFreeRefTable	(xmlRefTablePtr table);
int		xmlIsRef	(xmlDocPtr doc,
				 xmlNodePtr elem,
				 xmlAttrPtr attr);
int		xmlRemoveRef	(xmlDocPtr doc, xmlAttrPtr attr);

/**
 * The public function calls related to validity checking
 */

int		xmlValidateRoot		(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc);
int		xmlValidateElementDecl	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
		                         xmlElementPtr elem);
xmlChar *	xmlValidNormalizeAttributeValue(xmlDocPtr doc,
					 xmlNodePtr elem,
					 const xmlChar *name,
					 const xmlChar *value);
int		xmlValidateAttributeDecl(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
		                         xmlAttributePtr attr);
int		xmlValidateAttributeValue(xmlAttributeType type,
					 const xmlChar *value);
int		xmlValidateNotationDecl	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
		                         xmlNotationPtr nota);
int		xmlValidateDtd		(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
					 xmlDtdPtr dtd);
int		xmlValidateDtdFinal	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc);
int		xmlValidateDocument	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc);
int		xmlValidateElement	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
					 xmlNodePtr elem);
int		xmlValidateOneElement	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
		                         xmlNodePtr elem);
int		xmlValidateOneAttribute	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
					 xmlNodePtr	elem,
					 xmlAttrPtr attr,
					 const xmlChar *value);
int		xmlValidateDocumentFinal(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc);
int		xmlValidateNotationUse	(xmlValidCtxtPtr ctxt,
					 xmlDocPtr doc,
					 const xmlChar *notationName);
int		xmlIsMixedElement	(xmlDocPtr doc,
					 const xmlChar *name);
xmlAttributePtr	xmlGetDtdAttrDesc	(xmlDtdPtr dtd,
					 const xmlChar *elem,
					 const xmlChar *name);
xmlNotationPtr	xmlGetDtdNotationDesc	(xmlDtdPtr dtd,
					 const xmlChar *name);
xmlElementPtr	xmlGetDtdElementDesc	(xmlDtdPtr dtd,
					 const xmlChar *name);

int		xmlValidGetValidElements(xmlNode *prev,
					 xmlNode *next,
					 const xmlChar **list,
					 int max);
int		xmlValidGetPotentialChildren(xmlElementContent *ctree,
					 const xmlChar **list,
					 int *len,
					 int max);
#ifdef __cplusplus
}
#endif
#endif /* __XML_VALID_H__ */
