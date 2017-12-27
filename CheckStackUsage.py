import re
import glob
import sys
import os

if len(sys.argv)==2 and os.path.isdir(sys.argv[1]):
	AsmFiles=glob.glob(os.path.join(sys.argv[1],'*.asm'))
elif len(sys.argv)>=2:
	AsmFiles=sys.argv[1:]
else:
	print >> sys.stderr, "Syntax:"
	print >> sys.stderr, "  CheckStackUsage.py <CompilationDirectory>"
	print >> sys.stderr, "or"
	print >> sys.stderr, " CheckStackUsage.py <AsmFile1> <AsmFile2> ..."
	sys.exit(1)

RePatternFunctionBegin = re.compile(";Allocation info for local variables in function '(.*)'")
RePatternFunctionEnd = re.compile(";\s*function\s+.*\s*$")
RePatternStackAllocation = re.compile(";\s*(\w+)\s*Allocated to stack - _bp ([-+]\d+)")
RePatternLCall = re.compile("\slcall\s_{0,1}([^\s]+)\s*$",re.IGNORECASE)

Calls={}
FunctionStackUsage={}

print 'Analyzed assember files:'
for fname in AsmFiles:
	print '\t'+fname
	ActifFunction=""
	with open(fname) as f:
		for line in f:
			ReResult = RePatternFunctionBegin.search(line)
			if ReResult:
				ActifFunction=ReResult.group(1)
				MinStack=-2
				MaxStack=0
				Calls[ActifFunction]=[]
				#print ReResult.group(1)

			ReResult = RePatternFunctionEnd.search(line)
			if ReResult:
				#print "  "+ActifFunction+" : "+str(MaxStack)+","+str(MinStack)+"->"+str(MaxStack-MinStack)
				FunctionStackUsage[ActifFunction]=MaxStack-MinStack

			ReResult = RePatternStackAllocation.search(line)
			if ReResult:
				#print ReResult.group(1)+":"+ReResult.group(2)
				StackLevel=int(ReResult.group(2))
				#print StackLevel
				MinStack=min(MinStack,StackLevel)
				MaxStack=max(MaxStack,StackLevel)

			ReResult = RePatternLCall.search(line)
			if ReResult:
				Calls[ActifFunction].append(ReResult.group(1))
				#print "    "+ReResult.group(1)

print 'Sub-function call tree:'

def ReportFunction(Function,Stack,Level,NbrCalls):
	if FunctionStackUsage.get(Function)==None:
		Stack=Stack+2
		print '?\t'+str(Stack)+'\t'+('\t'*Level)+Function+' ('+str(NbrCalls)+'x)'
	else:
		Stack=Stack+FunctionStackUsage[Function]
		print str(FunctionStackUsage[Function])+'\t'+str(Stack)+'\t'+('\t'*Level)+Function+' ('+str(NbrCalls)+'x)'
		for SubFunction in list(set(Calls[Function])):
			ReportFunction(SubFunction,Stack,Level+1,Calls[Function].count(SubFunction))

for Function in ['setup','loop']:
	ReportFunction(Function,0,0,1)