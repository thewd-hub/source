//////////////////////////////////////////////
// InstallShield Java3 Password Cracker     //
// (C)thewd, thewd@hotmail.com              //
//////////////////////////////////////////////
//////////////////////////////////////////////
// Bypass Password Checks for installations //
// created with InstallShield Java v3.xx    //
//                                          //
// Tested with...                           //
// - v3.00.013, v3.00.040, v3.00.043        //
//   v3.00.080, v3.00.115 (Release Version) //
// - v3.01.125, v3.01.132 (Release Version) //
// - v3.02.135 (Release Version)            //
// - v3.03.146 (Release Version)            //
// - v3.50.145 (Release Version)            //
//////////////////////////////////////////////
import java.io.File;
import java.lang.reflect.Array;
import java.lang.reflect.Method;

public class ISjava3
{
	//////////////////////////////////////////////////////////
	// InstallShield ArchiveClassLoader.java Customising... //
	//                                                      //
	// Alter Constructor - moved instantiateArchiveReader   //
	// to custom loader.                                    //
	//                                                      //
	// Create new method to set the installation filename,  //
	// public void setArchiveFilename(String fileName)      //
	//                                                      //
	// Remove archiveFilename assignment in                 //
	// instantiateArchiveReader method and move             //
	// loadOffsetTable to custom loader.                    //
	//                                                      //
	// Remove main method.                                  //
	//                                                      //
	// getWizardID returns null if error occurs.            //
	//                                                      //
	// Change from private to public - getWizardID,         //
	// instantiateArchiveReader, loadOffsetTable            //
	//////////////////////////////////////////////////////////
	static final String applicationTitle = "InstallShield Java3 Password Cracker";
	static final String applicationVersion = "1.30";
	static final String copyrightInfo = "(C)thewd, thewd@hotmail.com";

	static final int java3BuildUnknown = 0;
	static final int java300Build13 = 1;
	static final int java300Build40 = 2;
	static final int java300Build43 = 3;
	static final int java300Build80 = 4;
	static final int java300Build115 = 5;
	static final int java301Build125 = 6;
	static final int java301Build132 = 7;
	static final int java302Build135 = 8;
	static final int java303Build146 = 9;
	static final int java350Build145 = 10;

	private String errorMessages[];

	// initialise the error messages
	private void initErrorMessages()
	{
		errorMessages = new String[8];

		errorMessages[0] = "Unable to open or read the InstallShield Java3 archive";
		errorMessages[1] = "Unable to open or read the ISjava3 Support archive";
		errorMessages[2] = "Unable to determine version of the InstallShield archive";
		errorMessages[3] = "Error occurred while initialising ArchiveClassLoader";
		errorMessages[4] = "Only Archives created with InstallShield Java3 are supported";
		errorMessages[5] = "Error occurred while executing InstallShield Wizard";
		errorMessages[6] = "Unable to open or read the ArchiveClassLoader class";
		errorMessages[7] = "Unable to open or read the PasswordPanel class";
	}

	// display the error message errorNumber and terminates
	private void displayErrorMessage(int errorNumber, boolean exitProgram)
	{
		initErrorMessages();
		if (errorMessages[errorNumber] != null) System.out.println("\nError #" + errorNumber + ": " + errorMessages[errorNumber]);
		if (exitProgram) System.exit(1);
	}

	// display application Title and associated information
	private static void displayTitle()
	{
		System.out.print(applicationTitle);
		System.out.println(" v" + applicationVersion);
		System.out.println(copyrightInfo);
	}

	// display valid options required for application execution
	private static void displayOptions()
	{
		System.out.println("\nUsage:\tjava -jar ISjava3.jar <java3 archive>");
		System.exit(0);
	}

	// attempt to find a public method within a java class
	private Method findMethod(Class classFileName, String methodName)
	{
		try
		{
			Method allMethods[] = classFileName.getMethods();
			for (int i = 0; i < allMethods.length; i++)
			{
				Method currentMethod = allMethods[i];
				if (currentMethod.getName().equals(methodName)) return currentMethod;
			}
		}
		catch (Exception e)
		{
		}
		return null;
	}

	// load the custom version of the PasswordPanel
	// before the original one (Accepts Any Password)
	private byte[] loadPasswordPanel(String jarFileName, String className, String baseName)
	{
		String classFileName = className.replace('.', '/') + ".class";
		JarResources jarRes = new JarResources(jarFileName);
		byte[] fileBuffer = jarRes.getResource(baseName + classFileName);

		if ((className != null) && (fileBuffer != null)) return fileBuffer;
		else displayErrorMessage(7, true);
		return null;
	}

	// load the custom version of InstallShield ArchiveClassLoader
	private Class loadArchiveClassLoader(String jarFileName, String className, String baseName)
	{
		FileClassLoader fcl = new FileClassLoader();
		String classFileName = className.replace('.', '/') + ".class";

		Class archiveClass = fcl.loadClassFromJar(jarFileName, baseName + classFileName);

		if (archiveClass != null) return archiveClass;
		else displayErrorMessage(6, true);
		return null;
	}

	// begin the Custom Java Installation Wizard
	private void beginJavaInstallation(String[] args)
	{
		displayTitle();

		if (args.length <= 0) displayOptions();

		String classFileName = args[args.length - 1];
		if (!classFileName.endsWith(".class")) classFileName += ".class";

		// check that the InstallShield Archive exists
		File archiveFile = new File(classFileName);
		if (!archiveFile.exists()) displayErrorMessage(0, true);

		// check that the ISjava3support.jar archive exists
		String jarFileName = "ISjava3support.jar";
		File jarFile = new File(jarFileName);
		if (!jarFile.exists()) displayErrorMessage(1, true);

		int buildVersion = java3BuildUnknown;

		// attempt to determine build version of archive
		try
		{
			FileClassLoader fcl = new FileClassLoader();
			Class archiveClass = fcl.loadClassFromFile(classFileName);

			// all InstallShield Archives should have this method
			Method instantiateArchiveReader = archiveClass.getDeclaredMethod("instantiateArchiveReader", null);

			if (instantiateArchiveReader != null)
			{
				buildVersion = java300Build13;

				// only build versions >~ 13 have this method
				Method showSplashScreen = archiveClass.getDeclaredMethod("showSplashScreen", null);
				if (showSplashScreen != null)
				{
					//buildVersion = java300Build40;
					//buildVersion = java300Build43;
					buildVersion = java300Build80;

					// only build version >~ 80 have this method
					Method findBytesInConstantsPool = archiveClass.getDeclaredMethod("findBytesInConstantsPool", new Class[]{Array.newInstance(Byte.TYPE, 0).getClass(), Array.newInstance(Byte.TYPE, 0).getClass()});
					if (findBytesInConstantsPool != null)
					{
						//buildVersion = java300Build115;
						//buildVersion = java301Build125;
						//buildVersion = java301Build132;
						//buildVersion = java302Build135;
						//buildVersion = java303Build146;
						buildVersion = java350Build145;
					}
				}
			}
		}
		catch (Exception e)
		{
		}

		// unable to determine version
		if (buildVersion == java3BuildUnknown) displayErrorMessage(2, true);

		String baseName = null;
		String archiveLoader = null;
		String passwordPanel = null;

		if (buildVersion == java300Build13)
		{
			baseName = "v3.00.013/";
			archiveLoader = "merlin.wizards.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizards.panels.PasswordPanel";
		}
		if (buildVersion == java300Build40)
		{
			baseName = "v3.00.040/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java300Build43)
		{
			baseName = "v3.00.043/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java300Build80)
		{
			baseName = "v3.00.080/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java300Build115)
		{
			baseName = "v3.00.115/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java301Build125)
		{
			baseName = "v3.01.125/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java301Build132)
		{
			baseName = "v3.01.132/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java302Build135)
		{
			baseName = "v3.02.135/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java303Build146)
		{
			baseName = "v3.03.146/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}
		if (buildVersion == java350Build145)
		{
			baseName = "v3.50.145/";
			archiveLoader = "merlin.wizard.builder.ArchiveClassLoader";
			passwordPanel = "merlin.wizard.panels.PasswordPanel";
		}

		Class archiveClass;
		Object archiveClassInstance = null;
		Method loadOffsetTable = null;
		Method getWizardID = null;
		Method registerClass= null;
		Method instantiateArchiveReader = null;
		Method setArchiveFilename = null;

		// find and initialise ArchiveClassLoader class
		archiveClass = loadArchiveClassLoader(jarFileName, archiveLoader, baseName);
		if (archiveClass == null) displayErrorMessage(3, true);

		try
		{
			// find methods within custom version of ArchiveClassLoader
			archiveClassInstance = archiveClass.newInstance();
			loadOffsetTable = findMethod(archiveClass, "loadOffsetTable");
			getWizardID = findMethod(archiveClass, "getWizardID");
			registerClass = findMethod(archiveClass, "registerClass");
			instantiateArchiveReader = findMethod(archiveClass, "instantiateArchiveReader");
			setArchiveFilename = findMethod(archiveClass, "setArchiveFilename");
		}
		catch (Exception e)
		{
			displayErrorMessage(3, true);
		}

		String wizardID = null;

		try
		{
			Object archiveArgs[] = new Object[1];
			archiveArgs[0] = null;

			archiveArgs[0] = classFileName;
			setArchiveFilename.invoke(archiveClassInstance, archiveArgs);
			loadOffsetTable.invoke(archiveClassInstance, null);
			wizardID = (String)getWizardID.invoke(archiveClassInstance, null);
		}
		catch (Exception e)
		{
			displayErrorMessage(3, true);
		}

		// check if InstallShield Java Archive is v3.x
		if ((wizardID == null) || (!wizardID.startsWith("3.")))
		{
			displayErrorMessage(4, true);
		}

		System.out.println("\nRunning InstallShield Java3 Archive (using " + (baseName.replace('/', ' ')) + "classes)...");

		try
		{
			// load custom PasswordPanel and register it
			Object archiveArgs[] = new Object[2];
			archiveArgs[0] = passwordPanel;
			archiveArgs[1] = loadPasswordPanel(jarFileName, passwordPanel, baseName);
			registerClass.invoke(archiveClassInstance, archiveArgs);

			// begin InstallShield Wizard
			instantiateArchiveReader.invoke(archiveClassInstance, null);
		}
		catch (Exception e)
		{
			displayErrorMessage(5, true);
		}
	}

	public static void main(String[] args)
	{
		ISjava3 is3 = new ISjava3();
		is3.beginJavaInstallation(args);
	}
}