#include "mehari/Analysis/InstructionDependencyAnalysis.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/CFG.h"

#include <algorithm>

using namespace llvm;


static cl::opt<bool> Verbose ("v", cl::desc("Enable verbose output to get more information."));


InstructionDependencyAnalysis::InstructionDependencyAnalysis() : FunctionPass(ID) {
	initializeInstructionDependencyAnalysisPass(*PassRegistry::getPassRegistry());
};

InstructionDependencyAnalysis::~InstructionDependencyAnalysis() {};


void InstructionDependencyAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.addRequiredTransitive<DependenceAnalysis>();
	AU.setPreservesAll();
}


bool InstructionDependencyAnalysis::runOnFunction(Function &func) {
	DA = &getAnalysis<DependenceAnalysis>();
	return false;
}


InstructionDependencyList InstructionDependencyAnalysis::getDependencies(Function &func) {

	// create list containing all instructions of the current function
	std::vector<Instruction*> instructions;
	for (inst_iterator I = inst_begin(func), E = inst_end(func); I != E; ++I)
		instructions.push_back(&*I);
	int instrCount = instructions.size();

	InstructionDependencyList dependencies;

	// dependencies between instructions:
	//  - data dependences on SSA registers -> use-def chain: all instructions that are used by the current instruction
	//  - data dependences on memory locations -> DependenceAnalysis: memory dependencies between instructions
	//	- control dependencies -> branch dependencies between basic blocks, unconditional branches and return statements


	// loop over all instructions and determine data dependencies
	// ----------------------------------------------------------
	for (int i=0; i<instrCount; i++) {

		Instruction *instr = instructions[i];
	
		if (Verbose)
			errs() << *instr << "\n";

		InstructionDependencies currentDependencies;

		// evaluate use-def chain to get all instructions the current instruction depends on
		for (User::op_iterator opIt = instr->op_begin(); opIt != instr->op_end(); ++opIt) {
			Instruction *opInstr = dyn_cast<Instruction>(*opIt);
			int instrNumber = getInstructionNumber(instructions, opInstr);
			if (instrNumber >= 0) {
				currentDependencies.push_back(instrNumber);
				if (Verbose)
					errs() << "USE-DEF: " << *instr << " depends on " << *opInstr << "\n";
			}
		}
		
		// run the DependenceAnalysis on all instructions that access memory
		// to determine dependencies store->load
		if (instr->mayReadFromMemory() || instr->mayWriteToMemory()) {
			for (int j=0; j<i; j++) {
				Instruction *currentInstr = instructions[j];
				if (currentInstr->mayReadFromMemory() || currentInstr->mayWriteToMemory()) {
					Dependence *dep = DA->depends(currentInstr, instr, true);
					if (dep != NULL) {
						std::string srcOpcode = dep->getSrc()->getOpcodeName();
						std::string dstOpcode = dep->getDst()->getOpcodeName();						
						if (srcOpcode == "store" && dstOpcode == "load") 
							// compare operands for store->load operations
							if (dep->getSrc()->getOperand(1) == dep->getDst()->getOperand(0)) {
								// add depedency
								currentDependencies.push_back(j);
								if (Verbose)
									errs() << "MEM-DEP: " << *(dep->getDst()) << "  depends on  " << *(dep->getSrc()) << "\n";
							}
					}
				}
			}
		}
		dependencies.push_back(currentDependencies);
	}


	// loop over all basic blocks and determine control flow dependencies
	// ------------------------------------------------------------------
	for (Function::iterator bi = func.begin(), be = func.end(); bi != be; ++bi) {
		BasicBlock* bb = dyn_cast<BasicBlock>(&*bi);

		// if the last instruction of the basic block is an unconditional branch
		// or an return statement add a dependency to the previous instruction 
		// in the same basic block to assure the control flow
		Instruction *lastInstr = bb->getTerminator();
		if (BranchInst *br = dyn_cast<BranchInst>(lastInstr)) {
    	if (br->isUnconditional()) {
    		if (bb->size() > 1) {
	    		// add dependency if the unconditional branch is not 
	    		// the only instruction in this basic block
	    		int currentInstrNumber = getInstructionNumber(instructions, lastInstr);
	    		dependencies[currentInstrNumber].push_back(currentInstrNumber-1);
    		}
    	}
    }
    else if (ReturnInst *ret = dyn_cast<ReturnInst>(lastInstr)) {
    	if (ret->getReturnValue() == NULL) {
    		int currentInstrNumber = getInstructionNumber(instructions, lastInstr);
    		if (currentInstrNumber != 0)
    			// add dependency if the return instruction is not 
    			// the only instruction in this function
    			dependencies[currentInstrNumber].push_back(currentInstrNumber-1);
    	}
  	}

  	// loop over all predecessors of the current basic block and add control dependencies
		for (pred_iterator pi = pred_begin(bb), pe = pred_end(bb); pi != pe; ++pi) {
			BasicBlock *pbb = *pi;
			// add a dependency between the first instruction of the current BB 
			// and the last instruction of the first predecessor BB
			if (Verbose)
				errs() << "CTL-DEP: " << (bb->front()) << "  depends on  " << (pbb->back()) << "\n";
			int tgtNumber = getInstructionNumber(instructions, &(bb->front()));
			int srcNumber = getInstructionNumber(instructions, &(pbb->back()));
			dependencies[tgtNumber].push_back(srcNumber);
		}
	}


	// sort dependencies and remove duplicate entries
	for (InstructionDependencyList::iterator depIt = dependencies.begin(), depEnd = dependencies.end(); depIt != depEnd; ++depIt) {
		std::sort(depIt->begin(), depIt->end());
		InstructionDependencies::iterator lastIt = std::unique(depIt->begin(), depIt->end());
		depIt->resize( std::distance(depIt->begin(), lastIt) );
	}

	return dependencies;
}


int InstructionDependencyAnalysis::getInstructionNumber(std::vector<Instruction*> &instructionList, Instruction *instruction) {
	std::vector<Instruction*>::iterator itPos = std::find(instructionList.begin(), instructionList.end(), &*instruction);
	if (itPos != instructionList.end())
		return int(itPos-instructionList.begin());
	return -1;
}


// register pass so we can call it using opt
char InstructionDependencyAnalysis::ID = 0;

INITIALIZE_PASS_BEGIN(InstructionDependencyAnalysis, "instrdep", 
	"Analysis of the dependencies between the instructions inside a function", true, true)
INITIALIZE_PASS_DEPENDENCY(DependenceAnalysis)
INITIALIZE_PASS_END(InstructionDependencyAnalysis, "instrdep", 
	"Analysis of the dependencies between the instructions inside a function", true, true)

// static RegisterPass<InstructionDependencyAnalysis>
// Y("instrdep", "Analysis of the dependencies between the instructions inside a function.");
