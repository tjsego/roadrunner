//
// Created by Ciaran on 25/10/2021.
//

#include <GetValueCodeGenBase.h>
#include "llvm/LLJit.h"
#include "gtest/gtest.h"
#include "TestModelFactory.h"
#include "rrLogger.h"
#include "rrRoadRunnerOptions.h"
#include "JitTests.h"
#include "rrConfig.h"
#include "rrUtils.h"

using namespace rr;
using namespace rrllvm;

class LLJitTests : public JitTests {
public:
    LLJitTests() {
        Logger::setLevel(Logger::LOG_DEBUG);
    };

};

using fibonacciFnPtr = int (*)(int);

TEST_F(LLJitTests, ModuleNonOwningNotNull) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    ASSERT_TRUE(llJit.getModuleNonOwning());
}

TEST_F(LLJitTests, ContextNonOwningNotNull) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    ASSERT_TRUE(llJit.getContextNonOwning());
}

TEST_F(LLJitTests, BuilderNonOwningNotNull) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    ASSERT_TRUE(llJit.getBuilderNonOwning());
}

TEST_F(LLJitTests, CreateJittedFibonacci) {
    // maybe the module and shouldnt be owned by the jit?:?
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    CreateFibFunction(llJit.getModuleNonOwning());
    std::cout << llJit.emitToString();
    llJit.addIRModule();
    fibonacciFnPtr fibPtr = (int (*)(int)) llJit.lookupFunctionAddress("fib");
    ASSERT_EQ(fibPtr(4), 3);
}

TEST_F(LLJitTests, FibonacciCached) {
    // maybe the module and shouldnt be owned by the jit?:?
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    llJit.setModuleIdentifier("fibMod");
    CreateFibFunction(llJit.getModuleNonOwning());
//    std::cout << llJit.emitToString();
    llJit.addIRModule();
    fibonacciFnPtr fibPtr1 = (int (*)(int)) llJit.lookupFunctionAddress("fib");
    ASSERT_EQ(fibPtr1(4), 3);

    std::unique_ptr<llvm::LLVMContext> ctx = std::make_unique<llvm::LLVMContext>();
    auto mod = std::make_unique<llvm::Module>("fibMod", *ctx);
    mod->setModuleIdentifier("fibMod");

    std::unique_ptr<llvm::MemoryBuffer> obj = SBMLModelObjectCache::getObjectCache().getObject(mod.get());
    LLJit llJit2(opt.modelGeneratorOpt);
    llJit2.addObjectFile(std::move(obj));

    fibonacciFnPtr fibPtr2 = (int (*)(int)) llJit2.lookupFunctionAddress("fib");
    ASSERT_EQ(fibPtr2(4), 3);

}

TEST_F(LLJitTests, LoadPowerFunction) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    auto fnPtr = (powFnTy) llJit.lookupFunctionAddress("pow");
    ASSERT_EQ(fnPtr(2, 2), 4);
}

TEST_F(LLJitTests, LoadQuotientFunction) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    auto fnPtr = (quotientFnTy) llJit.lookupFunctionAddress("quotient");
    ASSERT_EQ(fnPtr(4, 2), 2);
}

TEST_F(LLJitTests, Loadrr_minFunction) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    auto fnPtr = (quotientFnTy) llJit.lookupFunctionAddress("rr_min");
    ASSERT_EQ(fnPtr(4, 2), 2);
}

TEST_F(LLJitTests, Loadrr_csr_matrix_get_nzFunction) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    auto addr = llJit.lookupFunctionAddress("csr_matrix_get_nz");
    ASSERT_TRUE(addr);
}

TEST_F(LLJitTests, SetModuleIdentifier) {
    LoadSBMLOptions opt;
    LLJit llJit(opt.modelGeneratorOpt);
    llJit.setModuleIdentifier("FibMod");
    ASSERT_STREQ("FibMod", llJit.getModuleNonOwning()->getModuleIdentifier().c_str());
}


/**
 * Not concerned with simulation accuracy here, only that the sbml
 * compiles and the model simulates.
 */
TEST_F(LLJitTests, CheckModelSimulates) {
//    rr::Config::setValue(rr::Config::LLVM_BACKEND, rr::Config::LLVM_BACKEND_VALUES::MCJIT);
    rr::Config::setValue(rr::Config::LLVM_BACKEND, rr::Config::LLVM_BACKEND_VALUES::LLJIT);
    RoadRunner rr(OpenLinearFlux().str());
    auto data = rr.simulate(0, 10, 11);
    ASSERT_EQ(typeid(*data), typeid(ls::Matrix<double>));
}


TEST_F(LLJitTests, GetObjectFromCacheUsingOriginalString) {
//    rr::Config::setValue(rr::Config::LLVM_BACKEND, rr::Config::LLVM_BACKEND_VALUES::MCJIT);
    rr::Config::setValue(rr::Config::LLVM_BACKEND, rr::Config::LLVM_BACKEND_VALUES::LLJIT);
    RoadRunner rr(OpenLinearFlux().str());
    auto &cache = rrllvm::SBMLModelObjectCache::getObjectCache();
    cache.inspect();
    const std::string &sbml = rr.getSBML();
    std::string md5 = rr::getMD5(OpenLinearFlux().str());
    LLVMContext ctx;
    auto m = std::make_unique<llvm::Module>(md5, ctx);

    ASSERT_TRUE(m && "module loaded from LLJit cache is nullptr");
    /**
     * Test will segfault here if no model was found.
     */
    std::unique_ptr<llvm::MemoryBuffer> objectBuf = cache.getObject(m.get());
    ASSERT_TRUE(objectBuf);

    Expected<std::unique_ptr<llvm::object::ObjectFile>>
            objFile = llvm::object::ObjectFile::createObjectFile(objectBuf->getMemBufferRef());

    if (!objFile) {
        //LS DEBUG:  find a way to get the text out of the error.
        auto err = objFile.takeError();
        std::string s = "LLVM object supposed to be file, but is not.";
        rrLog(Logger::LOG_FATAL) << s;
        throw_llvm_exception(s);
    }
    LLJit llJit(LoadSBMLOptions().modelGeneratorOpt);
    llJit.addObjectFile(std::move(*objFile));



    // todo more assertions?

}

/**
 * This fails at the moment because we've modified the sbml md5
 * post loading, so the original md5 is the key to the cache while
 * rr.getSBML() now contains a different hash.
 */
TEST_F(LLJitTests, DISABLED_GetObjectFromCacheUsingRoadRunnerMethods) {
//    rr::Config::setValue(rr::Config::LLVM_BACKEND, rr::Config::LLVM_BACKEND_VALUES::MCJIT);
    rr::Config::setValue(rr::Config::LLVM_BACKEND, rr::Config::LLVM_BACKEND_VALUES::LLJIT);
    RoadRunner rr(OpenLinearFlux().str());
    auto &cache = rrllvm::SBMLModelObjectCache::getObjectCache();
    cache.inspect();
    const std::string &sbml = rr.getSBML();
    std::string md5 = rr::getMD5(rr.getSBML());
    LLVMContext ctx;
    auto m = std::make_unique<llvm::Module>(md5, ctx);

    /**
     * Test will segfault here if no model was found.
     */
    std::unique_ptr<llvm::MemoryBuffer> objectBuf = cache.getObject(m.get());
    ASSERT_TRUE(objectBuf);

    Expected<std::unique_ptr<llvm::object::ObjectFile>>
            objFile = llvm::object::ObjectFile::createObjectFile(objectBuf->getMemBufferRef());

    if (!objFile) {
        //LS DEBUG:  find a way to get the text out of the error.
        auto err = objFile.takeError();
        std::string s = "LLVM object supposed to be file, but is not.";
        rrLog(Logger::LOG_FATAL) << s;
        throw_llvm_exception(s);
    }
    LLJit llJit(LoadSBMLOptions().modelGeneratorOpt);
    llJit.addObjectFile(std::move(*objFile));
}


