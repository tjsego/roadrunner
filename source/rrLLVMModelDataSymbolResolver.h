/*
 * rrLLVMModelDataSymbolResolver.h
 *
 *  Created on: Jul 25, 2013
 *      Author: andy
 */

#ifndef rrLLVMModelDataSymbolResolver_H_
#define rrLLVMModelDataSymbolResolver_H_

#include "rrLLVMCodeGen.h"
#include "rrLLVMIncludes.h"
#include "rrLLVMModelDataSymbols.h"
#include "rrLLVMModelSymbols.h"

namespace libsbml
{
class Model;
}

namespace rr
{

/**
 * A terminal symbol resolver, which resolved everything
 * to values stored in the model data structure.
 *
 * terminal symbol resolvers treat all species as amounts.
 */
class LLVMModelDataSymbolResolver: public LLVMSymbolResolver
{
public:
    LLVMModelDataSymbolResolver(llvm::Value *modelData,
            const libsbml::Model *model,
            const LLVMModelDataSymbols &modelDataSymbols,
            llvm::IRBuilder<> &builder);

    virtual ~LLVMModelDataSymbolResolver();


    /**
     * The runtime resolution of symbols first search through the
     * replacement rules, applies them, them pulls the terminal
     * symbol values from the ModelData struct.
     *
     * The initial assigment generator overrides this and pulls
     * the terminal values from the initial values and assigments
     * specified in the model.
     */
    virtual llvm::Value *symbolValue(const std::string& symbol);

private:

    llvm::Value *modelData;
    const libsbml::Model *model;
    const LLVMModelDataSymbols &modelDataSymbols;
    llvm::IRBuilder<> &builder;
};

} /* namespace rr */
#endif /* rrLLVMModelDataSymbolResolver_H_ */
