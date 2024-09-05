package d6g.timestamping;


simple TimeTagger {
    @display("i=block/blackboard");
    @class(TimeTagger);
    string clockModule = default(""); // Relative path of a module that implements IClock; optional
}
