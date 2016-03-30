package es.hpcn.wormhole;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;

import backtype.storm.StormSubmitter;

public class StormRun
{
	private static String WHgetRunningClass()
	{
		StackTraceElement[] elems = Thread.currentThread().getStackTrace();

		StackTraceElement elem = elems[elems.length - 1];

		return elem.getClassName();
	}

	public static void main(String[] args) throws Exception
	{
		if (args.length > 0) {
			try {
				Class<?> c = Class.forName(args[0]);
				Class[] argTypes = new Class[] { String[].class };
				Method main = c.getDeclaredMethod("main", argTypes);
				String[] mainArgs = Arrays.copyOfRange(args, 1, args.length);
				System.out.format("invoking %s.main()%n", c.getName());

				//setsUp Einstein
				StormSubmitter.setEinsteinNode(args[0], mainArgs);

				main.invoke(null, (Object)mainArgs);

				// production code should handle these exceptions more gracefully

			} catch (ClassNotFoundException x) {
				System.out.println("Class " + args[0] + " not found.");

			} catch (NoSuchMethodException x) {
				System.out.println("Method \"main\" not found in class \"" + args[0] + "\".");

			} catch (IllegalAccessException x) {
				x.printStackTrace();

			} catch (InvocationTargetException x) {
				x.printStackTrace();
			}

		} else {
			System.out.println("Please, execute your program as: java -cp libjavaworm.jar " + WHgetRunningClass() + " <your storm class with Main method> [other params]");
		}
	}

}