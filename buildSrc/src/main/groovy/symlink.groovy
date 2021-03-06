import org.gradle.api.InvalidUserDataException;
import org.gradle.api.internal.file.FileResolver;
import org.gradle.api.internal.file.copy.CopyAction;
import org.gradle.api.internal.file.copy.CopySpecInternal;
import org.gradle.api.internal.file.copy.DestinationRootCopySpec;
import org.gradle.api.internal.file.copy.FileCopyAction;
import org.gradle.internal.reflect.Instantiator;

import org.gradle.api.internal.file.CopyActionProcessingStreamAction;
import org.gradle.api.internal.file.FileResolver;
import org.gradle.api.internal.tasks.SimpleWorkResult;
import org.gradle.api.tasks.WorkResult;

import org.gradle.api.internal.file.copy.CopyActionProcessingStream;
import org.gradle.api.internal.file.copy.FileCopyDetailsInternal;
import org.gradle.api.logging.Logger;
import org.gradle.util.Path;
import org.gradle.api.tasks.Copy;
import org.gradle.api.GradleException

import java.io.File;

public class FileLinkAction implements CopyAction {
	private final FileResolver fileResolver;
	private final overwriteExisting;
	private final Logger logger;
	private final Set<String> done;

	public FileLinkAction(FileResolver fileResolver, boolean overwrite, Logger logger) {
		this.fileResolver = fileResolver;
		this.overwriteExisting = overwrite;
		this.logger = logger;
		this.done = new HashSet<String>();
	}

	public WorkResult execute(CopyActionProcessingStream stream) {
		FileCopyDetailsInternalAction action = new FileCopyDetailsInternalAction();
		stream.process(action);
		return new SimpleWorkResult(action.didWork);
	}

	private boolean parentHasBeenSymlinked(File target) {
		while (target) {
			if (done.contains(target.getCanonicalPath()))
				return true;
			target = target.getParentFile();
		}
	}

	public static String getRelativePath(File target, File relativeTo) {
		target = target.getAbsoluteFile();
		relativeTo = relativeTo.getAbsoluteFile();

		Map<String, String> parents = new HashMap<String, String>();
		String rest = "";
		while (relativeTo != null) {
			parents.put(relativeTo.toString(), rest);

			if (rest.equals(""))
				rest = "..";
			else
				rest = ".." + File.separator + rest;
			relativeTo = relativeTo.getParentFile();
		}

		rest = "";
		while (target != null) {
			if (parents.containsKey(target.toString())) {
				String result = parents[target.toString()];
				if (result.equals("") && rest.equals(""))
					return ".";
				else if (result.equals(""))
					return rest;
				else if (!rest.equals(""))
					result += File.separator + rest;
				return result;
			}

			if (rest.equals(""))
				rest = target.getName();
			else
				rest = target.getName() + File.separator + rest;
			target = target.getParentFile();
		}

		return target;
	}

	private boolean createSymbolicLink(FileCopyDetailsInternal details, File link) {
		if (link.exists()) {
			if (link.getCanonicalFile().equals(details.getFile())) {
				// up-to-date
				done.add(link.getCanonicalPath());
				return false;
			} else {
				if (!overwriteExisting)
					throw new GradleException("The target file '" + link + "' already exists.");
			}
		}

		if (parentHasBeenSymlinked(link))
			return false;

		String relativeSourcePath = getRelativePath(details.getFile(), link.getParentFile())
		if (["ln", "-sf", relativeSourcePath, link].execute().waitFor() != 0) {
			logger.warn("Could not create symbolic link at '" + link + "' (pointing to '"
				+ details.getFile().getAbsolutePath() + "'). I will try to copy the file instead of creating a link.");

			return details.copyTo(link);
		} else {
			done.add(link.getCanonicalPath());
			return true;
		}
	}

	private class FileCopyDetailsInternalAction implements CopyActionProcessingStreamAction {
		private boolean didWork;

		public void processFile(FileCopyDetailsInternal details) {
			File target = fileResolver.resolve(details.getRelativePath().getPathString());
			boolean changed = createSymbolicLink(details, target);
			if (changed) {
				didWork = true;
			}
		}
	}
}


public class SymLink extends Copy {
	public boolean overwrite = true;

	@Override
	protected CopyAction createCopyAction() {
		File destinationDir = getDestinationDir();
		if (destinationDir == null) {
			throw new InvalidUserDataException("No copy destination directory has been specified, use 'into' to specify a target directory.");
		}
		return new FileLinkAction(getServices().get(FileResolver.class).withBaseDir(destinationDir), overwrite, getLogger());
	}
}
