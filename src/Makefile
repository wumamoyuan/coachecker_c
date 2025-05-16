objects := coachecker.o precheck.o AABACTranslator.o AABACAbsRef.o AABACSlice.o AABACReader.o AABACWriter.o AABACInstance.o AABACRule.o AABACResult.o AABACUtils.o hashSet.o hashMap.o hashBasedTable.o bn_java.o test.o

coachecker: ${objects} ../lib/libccl.a
	cc -static ${objects} ../lib/libccl.a -o ../bin/coachecker 

clean:
	rm -rf ${objects} ../bin/coachecker