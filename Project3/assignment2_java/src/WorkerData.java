public class WorkerData {
    private Command command;
    private int value;

    public WorkerData(Command command, int value) {
        this.command = command;
        this.value = value;
    }

    public Command getCommand() {
        return command;
    }

    public void setCommand(Command command) {
        this.command = command;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }
}