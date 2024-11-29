public class Main {

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java Main <number of workers>");
            return;
        }

        String N = args[0];
        if (!isInt(N)) {
            System.out.println("Invalid number of workers!");
            return;
        }

        // Create master
        int n = Integer.parseInt(N);
        Master master = new Master(n);
        master.start();
    }

    private static boolean isInt(String str) {
        try {
            Integer.parseInt(str);
            return true;
        } catch (NumberFormatException e) {
            return false;
        }
    }
}
