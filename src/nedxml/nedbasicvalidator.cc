//==========================================================================
// nedbasicvalidator.cc - part of the OMNeT++ Discrete System Simulation System
//
// Contents:
//   class NEDBasicValidator
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2002 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "nederror.h"
#include "nedbasicvalidator.h"

// FIXME duplicate!!! also present in cppgenerator.cc
static struct { char *fname; int args; } known_funcs[] =
{
   /* <math.h> */
   {"fabs", 1},    {"fmod", 2},
   {"acos", 1},    {"asin", 1},    {"atan", 1},   {"atan2", 1},
   {"sin", 1},     {"cos", 1},     {"tan", 1},    {"hypot", 2},
   {"ceil", 1},    {"floor", 1},
   {"exp", 1},     {"pow", 2},     {"sqrt", 1},
   {"log",  1},    {"log10", 1},

   /* OMNeT++ */
   {"uniform", 2},      {"intuniform", 2},       {"exponential", 1},
   {"normal", 2},       {"truncnormal", 2},
   {"genk_uniform", 3}, {"genk_intuniform",  3}, {"genk_exponential", 2},
   {"genk_normal", 3},  {"genk_truncnormal", 3},
   {"min", 2},          {"max", 2},

   /* OMNeT++, to support expressions */
   {"bool_and",2}, {"bool_or",2}, {"bool_xor",2}, {"bool_not",1},
   {"bin_and",2},  {"bin_or",2},  {"bin_xor",2},  {"bin_compl",1},
   {"shift_left",2}, {"shift_right",2},
   {NULL,0}
};

inline bool strnotnull(const char *s)
{
    return s && s[0];
}

void NEDBasicValidator::checkUniqueness(NEDElement *node, int childtype, const char *attr)
{
    for (NEDElement *child1=node->getFirstChildWithTag(childtype); child1; child1=child1->getNextSiblingWithTag(childtype))
    {
        const char *attr1 = child1->getAttribute(attr);
        for (NEDElement *child2=node->getFirstChildWithTag(childtype); child2!=child1; child2=child2->getNextSiblingWithTag(childtype))
        {
            const char *attr2 = child2->getAttribute(attr);
            if (attr1 && attr2 && !strcmp(attr1,attr2))
                NEDError(child1, "name '%s' not unique",attr1);
        }
    }
}

void NEDBasicValidator::checkExpressionAttributes(NEDElement *node, const char *attrs[], bool optional[], int n)
{
    if (parsedExpressions)
    {
        // allow attribute values to be present, but check there are no
        // Expression children that are not in the list
        for (NEDElement *child=node->getFirstChildWithTag(NED_EXPRESSION); child; child=child->getNextSiblingWithTag(NED_EXPRESSION))
        {
            ExpressionNode *expr = (ExpressionNode *) child;
            const char *target = expr->getTarget();
            int i;
            for (i=0; i<n; i++)
                if (!strcmp(target, attrs[i]))
                    break;
            if (i==n)
                NEDError(child, "'expression' element with invalid target attribute '%s'",target);
        }
    }
    else
    {
        // check: should be no Expression children at all
        if (node->getFirstChildWithTag(NED_EXPRESSION))
            NEDError(node, "'expression' element found while using non-parsed expressions\n");
    }

    // check mandatory expressions are there
    for (int i=0; i<n; i++)
    {
       if (!optional[i])
       {
           if (parsedExpressions)
           {
               // check: Expression element must be there
               ExpressionNode *expr;
               for (expr=(ExpressionNode *)node->getFirstChildWithTag(NED_EXPRESSION); expr; expr=expr->getNextExpressionNodeSibling())
                   if (strnotnull(expr->getTarget()) && !strcmp(expr->getTarget(),attrs[i]))
                       break;
               if (!expr)
                   NEDError(node, "expression-valued attribute '%s' not present in parsed form (missing 'expression' element)", attrs[i]);
           }
           else
           {
               // attribute must be there
               if (!node->getAttribute(attrs[i]) || !(node->getAttribute(attrs[i]))[0])
                   NEDError(node, "missing attribute '%s'", attrs[i]);
           }
       }
    }
}

void NEDBasicValidator::validateElement(NedFilesNode *node)
{
}

void NEDBasicValidator::validateElement(NedFileNode *node)
{
}

void NEDBasicValidator::validateElement(ImportNode *node)
{
}

void NEDBasicValidator::validateElement(ImportedFileNode *node)
{
}

void NEDBasicValidator::validateElement(ChannelNode *node)
{
}

void NEDBasicValidator::validateElement(ChannelAttrNode *node)
{
    const char *expr[] = {"value"};
    bool opt[] = {false};
    checkExpressionAttributes(node, expr, opt, 1);
}

void NEDBasicValidator::validateElement(NetworkNode *node)
{
}

void NEDBasicValidator::validateElement(SimpleModuleNode *node)
{
}

void NEDBasicValidator::validateElement(CompoundModuleNode *node)
{
}

void NEDBasicValidator::validateElement(ParamsNode *node)
{
    // make sure parameter names are unique
    checkUniqueness(node, NED_PARAM, "name");
}

void NEDBasicValidator::validateElement(ParamNode *node)
{
}

void NEDBasicValidator::validateElement(GatesNode *node)
{
    // make sure gate names are unique
    checkUniqueness(node, NED_GATE, "name");
}

void NEDBasicValidator::validateElement(GateNode *node)
{
}

void NEDBasicValidator::validateElement(MachinesNode *node)
{
    // make sure machine names are unique
    checkUniqueness(node, NED_MACHINE, "name");
}

void NEDBasicValidator::validateElement(MachineNode *node)
{
}

void NEDBasicValidator::validateElement(SubmodulesNode *node)
{
    // make sure submodule names are unique
    checkUniqueness(node, NED_SUBMODULE, "name");
}

void NEDBasicValidator::validateElement(SubmoduleNode *node)
{
    const char *expr[] = {"vector-size"};
    bool opt[] = {true};
    checkExpressionAttributes(node, expr, opt, 1);

    // FIXME if there's a "like", name should be an existing module parameter name
}

void NEDBasicValidator::validateElement(SubstparamsNode *node)
{
    const char *expr[] = {"condition"};
    bool opt[] = {true};
    checkExpressionAttributes(node, expr, opt, 1);

    // make sure parameter names are unique
    checkUniqueness(node, NED_SUBSTPARAM, "name");
}

void NEDBasicValidator::validateElement(SubstparamNode *node)
{
    const char *expr[] = {"value"};
    bool opt[] = {false};
    checkExpressionAttributes(node, expr, opt, 1);
}

void NEDBasicValidator::validateElement(GatesizesNode *node)
{
    const char *expr[] = {"condition"};
    bool opt[] = {true};
    checkExpressionAttributes(node, expr, opt, 1);

    // make sure gate names are unique
    checkUniqueness(node, NED_GATESIZE, "name");
}

void NEDBasicValidator::validateElement(GatesizeNode *node)
{
    const char *expr[] = {"vector-size"};
    bool opt[] = {true};
    checkExpressionAttributes(node, expr, opt, 1);
}

void NEDBasicValidator::validateElement(SubstmachinesNode *node)
{
    const char *expr[] = {"condition"};
    bool opt[] = {true};
    checkExpressionAttributes(node, expr, opt, 1);
}

void NEDBasicValidator::validateElement(SubstmachineNode *node)
{
}

void NEDBasicValidator::validateElement(ConnectionsNode *node)
{
    // if check=true, make sure all gates are connected
    //FIXME
}

void NEDBasicValidator::validateElement(ConnectionNode *node)
{
    const char *expr[] = {"condition", "src-module-index", "src-gate-index", "dest-module-index", "dest-gate-index"};
    bool opt[] = {true, true, true, true, true};
    checkExpressionAttributes(node, expr, opt, 5);

    // FIXME make sure submodule and gate names are valid
    // FIXME gates & modules are really vector (or really not)
}

void NEDBasicValidator::validateElement(ConnAttrNode *node)
{
    const char *expr[] = {"value"};
    bool opt[] = {false};
    checkExpressionAttributes(node, expr, opt, 1);

    // if this is a "channel", expression must contain a single string constant!!!
}

void NEDBasicValidator::validateElement(ForLoopNode *node)
{
    // make sure loop variable names are unique
    checkUniqueness(node, NED_LOOP_VAR, "param-name");
}

void NEDBasicValidator::validateElement(LoopVarNode *node)
{
    const char *expr[] = {"from-value", "to-value"};
    bool opt[] = {false, false};
    checkExpressionAttributes(node, expr, opt, 2);
}

void NEDBasicValidator::validateElement(DisplayStringNode *node)
{
}

void NEDBasicValidator::validateElement(ExpressionNode *node)
{
}

void NEDBasicValidator::validateElement(OperatorNode *node)
{
    // check operator name is valid and argument count matches
    const char *op = node->getName();

    // next list uses space as separator, so make sure op does not contain space
    if (strchr(op, ' '))
    {
        NEDError(node, "invalid operator '%s' (contains space)",op);
        return;
    }

    // count arguments
    int args = 0;
    for (NEDElement *child=node->getFirstChild(); child; child=child->getNextSibling())
       args++;

    // unary?
    if (strstr("! ~",op))
    {
         if (args!=1)
            NEDError(node, "operator '%s' should have 1 operand, not %d", op, args);
    }
    // unary or binary?
    else if (strstr("-",op))
    {
         if (args!=1 && args!=2)
            NEDError(node, "operator '%s' should have 1 or 2 operands, not %d", op, args);
    }
    // binary?
    else if (strstr("+ * / % ^ == != > >= < <= && || ## & | # << >>",op))
    {
         if (args!=2)
            NEDError(node, "operator '%s' should have 2 operands, not %d", op, args);
    }
    // tertiary?
    else if (strstr("?:",op))
    {
         if (args!=3)
            NEDError(node, "operator '%s' should have 3 operands, not %d", op, args);
    }
    else
    {
        NEDError(node, "invalid operator '%s'",op);
    }
}

void NEDBasicValidator::validateElement(FunctionNode *node)
{
    // if we know the function, check argument count
    const char *func = node->getName();

    // count arguments
    int args = 0;
    for (NEDElement *child=node->getFirstChild(); child; child=child->getNextSibling())
       args++;

    // if it's an operator, treat specially
    if (!strcmp(func,"index"))
    {
         if (args!=0)
             NEDError(node, "operator 'index' does not take arguments");
         return;
    }
    else if (!strcmp(func,"sizeof"))
    {
         if (args!=1)
             NEDError(node, "operator 'sizeof' takes one argument");
         else if (node->getFirstChild()->getTagCode()!=NED_IDENT)
             NEDError(node, "argument of operator 'sizeof' should be an identifier");
         else
         {
             // FIXME further check it's a meaningful identifier!
         }
         return;
    }
    else if (!strcmp(func,"input"))
    {
         if (args>2)
             NEDError(node, "operator 'input' takes 0, 1 or 2 arguments");
         return;
    }

    // check if we know about it
    int i;
    for (i=0; known_funcs[i].fname!=NULL;i++)
        if (!strcmp(func,known_funcs[i].fname))
            break;
    if (known_funcs[i].fname!=NULL)
    {
        // found, check arg count matches
        if (known_funcs[i].args!=args)
            NEDError(node, "function '%s' should have %d operands, not %d", func, known_funcs[i].args, args);
    }
}

void NEDBasicValidator::validateElement(ParamRefNode *node)
{
    const char *expr[] = {"module-index", "param-index"};
    bool opt[] = {true, true};
    checkExpressionAttributes(node, expr, opt, 2);

    // make sure parameter exists
    //FIXME
}

void NEDBasicValidator::validateElement(IdentNode *node)
{
    // make sure ident (loop variable) exists
    //FIXME
}

void NEDBasicValidator::validateElement(ConstNode *node)
{
    // verify syntax of constant
    int type = node->getType();
    const char *text = node->getText();
    const char *value = node->getValue();

    if (type==NED_CONST_BOOL)
    {
        // check bool
        // FIXME TBD
    }
    else if (type==NED_CONST_INT)
    {
        // check int
        // FIXME TBD
    }
    else if (type==NED_CONST_REAL)
    {
        // check real
        // FIXME TBD
    }
    else if (type==NED_CONST_STRING)
    {
        // check string
        // FIXME TBD
    }
    else if (type==NED_CONST_TIME)
    {
        // check time
        // FIXME TBD
    }
}

void NEDBasicValidator::validateElement(CppincludeNode *node)
{
    // FIXME check filename syntax
}

void NEDBasicValidator::validateElement(CppStructNode *node)
{
}

void NEDBasicValidator::validateElement(CppCobjectNode *node)
{
}

void NEDBasicValidator::validateElement(CppNoncobjectNode *node)
{
}

void NEDBasicValidator::validateElement(EnumNode *node)
{
}

void NEDBasicValidator::validateElement(EnumFieldsNode *node)
{
}

void NEDBasicValidator::validateElement(EnumFieldNode *node)
{
}

void NEDBasicValidator::validateElement(MessageNode *node)
{
}

void NEDBasicValidator::validateElement(ClassNode *node)
{
}

void NEDBasicValidator::validateElement(StructNode *node)
{
}

void NEDBasicValidator::validateElement(FieldsNode *node)
{
}

void NEDBasicValidator::validateElement(FieldNode *node)
{
    // FIXME check syntax of default value
    // FIXME check type of default value agrees with field type
}

void NEDBasicValidator::validateElement(PropertiesNode *node)
{
}

void NEDBasicValidator::validateElement(PropertyNode *node)
{
    // FIXME check syntax of value
}


void NEDBasicValidator::validateElement(UnknownNode *node)
{
}

