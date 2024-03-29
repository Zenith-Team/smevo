.rodata
CoursePathFormat:       .string "course_res_pack/%s.sarc"
RDashCoursePathFormat:  .string "rdash://course_res_pack/%s.sarc"

.text

.global LoadLevelArchiveWithoutDecomp
LoadLevelArchiveWithoutDecomp:
    cmpwi     r3, 0x0
    bne       LoadLevelArchiveContinue

    lis       r4, nullterminator@ha
    addi      r3, r1, 0x10
    addi      r4, r4, nullterminator@l
    crclr     4*cr1+eq
    bl        setBufferedString

    lis       r12, sInstance__8RDashMgr@ha
    lwz       r12, sInstance__8RDashMgr@l(r12)
    lbz       r0, 0x11(r12)
    addi      r3, r1, 0x10
    lwz       r5, 0x0(r28)
    cmpwi     r0, 0x0
    bne       SetLevelArchiveRDash

    lis       r4, CoursePathFormat@ha
    addi      r4, r4, CoursePathFormat@l
    b         LoadLevelArchiveWithoutDecompContinue

SetLevelArchiveRDash:
    lis       r4, RDashCoursePathFormat@ha
    addi      r4, r4, RDashCoursePathFormat@l

LoadLevelArchiveWithoutDecompContinue:
    crclr     4*cr1+eq
    bl        formatBufferedString
    li        r7, 0x0
    mr        r6, r29
    addi      r5, r1, 0x10
    mr        r4, r28
    mr        r3, r27
    bl        ResMgr_LoadLevelResource
    cmpwi     r3, 0x0
    bne       LoadLevelArchiveContinue
    stw       r3, 0x10(r27)
    b         ResMgr_LoadLevelArchiveFail

LoadLevelArchiveContinue:
    stw       r3, 0x10(r27)
    b         ResMgr_LoadLevelArchiveSuccess
