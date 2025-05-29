import { useEffect, useState } from 'react';
import axios from 'axios';
import {
  Dialog, DialogTitle, DialogContent, DialogActions,
  Button, TextField, MenuItem, IconButton, Snackbar, Alert, Backdrop,
  CircularProgress
} from '@mui/material';
import UploadFileIcon from '@mui/icons-material/UploadFile';

function App() {
  const [repos, setRepos] = useState([]);
  const [selectedRepo, setSelectedRepo] = useState(null);
  const [files, setFiles] = useState([]);
  const [file, setFile] = useState(null);
  const [repoModalOpen, setRepoModalOpen] = useState(false);
  const [uploadModalOpen, setUploadModalOpen] = useState(false);
  const [newRepo, setNewRepo] = useState({ name: '', type: 'LocalStorage', config: {} });
  const [selectedFile, setSelectedFile] = useState(null);
  const [previewContent, setPreviewContent] = useState('');
  const [previewOpen, setPreviewOpen] = useState(false);
  const [previewTitle, setPreviewTitle] = useState('');
  const [toastMessage, setToastMessage] = useState('');
  const [toastType, setToastType] = useState('');
  const [toastOpen, setToastOpen] = useState(false);
  const [copyModalOpen, setCopyModalOpen] = useState(false);
  const [copyTargetRepo, setCopyTargetRepo] = useState('');
  const [loading, setLoading] = useState(false);

  const showToast = (type,msg) => {
    setToastMessage(msg);
    setToastType(type);
    setToastOpen(true);
  };

  const pluginFields = {
    LocalStorage: [],
    S3Storage: ['bucket', 'accessKey', 'secretKey'],
    HTTPStorage: ['url'],
    AzureStorage: ['container', 'connectionString'],
    GCSStorage: ['bucket', 'keyfile']
  };

  const fetchRepos = async () => {
      setLoading(true);
      try {
        const res = await axios.get('/api/repos/list');
        const json = typeof res.data === 'string' ? JSON.parse(res.data) : res.data;
        if (json.success) {
          setRepos(json.data);
        } else {
          showToast("error",json.error || 'Failed to fetch repo list');
          setRepos([]);
        }
      } catch (e) {
        showToast("error",'Error fetching repos');
      } finally {
        setLoading(false);
      }
  };

  const fetchFiles = async (repoSlug) => {
    setLoading(true);
    try {
      const res = await axios.get(`/api/repos/view/${repoSlug}`);
      const json = typeof res.data === 'string' ? JSON.parse(res.data) : res.data;
      if (json.success) {
        setFiles(json.data);
      } else {
        showToast("error",json.error || 'Failed to fetch files');
        setFiles([]);
      }
    } catch {
      showToast("error",'Error fetching files');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchRepos();
  }, []);

  const addRepo = async () => {
    setLoading(true);
    try {
      const payload = {
        name: newRepo.name,
        type: newRepo.type,
        config: newRepo.config
      };
      const res = await axios.post('/api/repos/add', payload);
      const json = res.data;
      if (json.success) {
        fetchRepos();
        setRepoModalOpen(false);
        showToast("success",json.data);
      } else {
        showToast("error",json.error || 'Failed to add repository');
      }
    } catch {
      showToast("error",'Error adding repository');
    } finally {
      setLoading(false);
    }
  };

  const upload = async () => {
    setLoading(true);
    const form = new FormData();
    form.append('file', file);
    form.append('repo', selectedRepo);
    try {
      const res = await axios.post('/api/repos/upload', form);
      const json = res.data;
      if (json.success) {
        fetchFiles(selectedRepo);
        setUploadModalOpen(false);
        showToast("success",json.data);
      } else {
        showToast("error",json.error || 'Upload failed');
      }
    } catch {
      showToast("error",'Error uploading file');
    } finally {
      setLoading(false);
    }
  };

  const previewFile = async (filename) => {
    setLoading(true);
    try {
      const res = await axios.get(`/api/repos/file/${selectedRepo}/${encodeURIComponent(filename)}`);
      const json = typeof res.data === 'string' ? JSON.parse(res.data) : res.data;
      if (json.success) {
        setPreviewTitle(filename);
        const { content, encoding, mime } = json.data;
        if (encoding=="base64" && mime.startsWith('image/')) {
          // Image preview
          setPreviewContent(<img src={`data:${mime};base64,${content}`} alt={filename} className="max-w-full max-h-[60vh]" />);
        } else if (encoding == "base64") {
          // Generic binary data (not image)
          setPreviewContent(<pre className="text-sm overflow-y-auto max-h-[60vh]">[Binary file - base64 encoded]
          {content}
          </pre>);
        } else {
          // Plain text
          setPreviewContent(<pre className="text-sm whitespace-pre-wrap overflow-y-auto max-h-[60vh]">{content}</pre>);
        }
        setPreviewOpen(true);
      } else {
        showToast("error",json.error || 'Failed to load file');
      }
    } catch (err) {
      showToast("error",'Error loading file');
    } finally {
      setLoading(false);
    }
  };
  
  

  return (
    <div className="h-screen flex flex-col font-sans">
      <Backdrop open={loading} sx={{ zIndex: (theme) => theme.zIndex.drawer + 1 }}>
        <CircularProgress color="inherit" />
      </Backdrop>
      <div className="bg-blue-700 text-white p-3 text-lg font-semibold shadow flex justify-between items-center">
        Repo Manager
        <div className="space-x-2">
          <Button variant="contained" size="small" onClick={() => setRepoModalOpen(true)}>+ Repo</Button>
          <IconButton color="inherit" onClick={() => setUploadModalOpen(true)} disabled={!selectedRepo}>
            <UploadFileIcon />
          </IconButton>
        </div>
      </div>

      <div className="flex flex-1">
        <aside className="w-64 bg-gray-100 border-r p-4 overflow-y-auto">
          <h2 className="font-bold mb-2">Repositories</h2>
          <ul>
            {repos.map(repo => (
              <li key={repo.slug}>
                <button
                  className={`w-full text-left px-2 py-1 rounded ${selectedRepo === repo.slug ? 'bg-blue-200' : 'hover:bg-gray-200'}`}
                  onClick={() => {
                    setSelectedRepo(repo.slug);
                    fetchFiles(repo.slug);
                  }}
                >
                  {repo.name}
                </button>
              </li>
            ))}
          </ul>
        </aside>

        <main className="flex-1 p-4 overflow-y-auto">
          <Snackbar open={toastOpen} autoHideDuration={5000} onClose={() => setToastOpen(false)}>
            <Alert severity={toastType} onClose={() => setToastOpen(false)} sx={{ width: '100%' }}>
              {toastMessage}
            </Alert>
          </Snackbar>

          <table className="w-full border text-sm">
            <thead className="bg-gray-100">
              <tr>
                <th className="text-left p-2 border">Filename</th>
                <th className="text-left p-2 border">MIME Type</th>
                <th className="text-left p-2 border">Size</th>
                <th className="text-left p-2 border">Modified</th>
              </tr>
            </thead>
            <tbody>
              {files.map(file => (
                <tr
                  key={file.name}
                  className={`cursor-pointer select-none ${file.name === selectedFile ? 'bg-blue-200' : 'hover:bg-gray-50'}`}
                  onClick={() => setSelectedFile(file.name)}
                  onDoubleClick={() => previewFile(file.name)}
                >
                  <td className="p-2 border">{file.name}</td>
                  <td className="p-2 border">{file.mime || '-'}</td>
                  <td className="p-2 border">{file.size || '-'}</td>
                  <td className="p-2 border">{file.modified || '-'}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </main>
      </div>

      <Dialog open={repoModalOpen} onClose={() => setRepoModalOpen(false)}>
        <DialogTitle>Add New Repository</DialogTitle>
        <DialogContent>
          <TextField
            label="Repository Name"
            fullWidth
            margin="dense"
            value={newRepo.name}
            onChange={e => setNewRepo({ ...newRepo, name: e.target.value })}
          />
          <TextField
            select
            label="Storage Type"
            fullWidth
            margin="dense"
            value={newRepo.type}
            onChange={e => {
              setNewRepo({ name: newRepo.name, type: e.target.value, config: {} });
            }}
          >
            <MenuItem value="LocalStorage">Local</MenuItem>
            <MenuItem value="S3Storage">S3</MenuItem>
            <MenuItem value="GCSStorage">GCS</MenuItem>
            <MenuItem value="AzureStorage">Azure</MenuItem>
            <MenuItem value="HTTPStorage">HTTP</MenuItem>
          </TextField>

          {pluginFields[newRepo.type]?.map(field => (
            <TextField
              key={field}
              label={field}
              fullWidth
              margin="dense"
              value={newRepo.config?.[field] || ''}
              onChange={e =>
                setNewRepo({
                  ...newRepo,
                  config: { ...newRepo.config, [field]: e.target.value }
                })
              }
            />
          ))}
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setRepoModalOpen(false)}>Cancel</Button>
          <Button onClick={addRepo} variant="contained">Add</Button>
        </DialogActions>
      </Dialog>

      <Dialog open={uploadModalOpen} onClose={() => setUploadModalOpen(false)}>
        <DialogTitle>Upload File</DialogTitle>
        <DialogContent>
          <input type="file" onChange={e => setFile(e.target.files[0])} />
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setUploadModalOpen(false)}>Cancel</Button>
          <Button onClick={upload} variant="contained" disabled={!file}>Upload</Button>
        </DialogActions>
      </Dialog>

      <Dialog open={previewOpen} onClose={() => setPreviewOpen(false)} fullWidth maxWidth="md">
        <DialogTitle>{previewTitle}</DialogTitle>
        <DialogContent dividers className="max-h-[70vh] overflow-y-auto">
          {previewContent}
        </DialogContent>
        <DialogActions>
        {selectedRepo && repos.find(r => r.slug === selectedRepo)?.type !== 'LocalStorage' && (
          <Button onClick={() => setCopyModalOpen(true)} color="primary">
            Download to Local
          </Button>
        )}
        <Button onClick={() => setPreviewOpen(false)}>Close</Button>
        </DialogActions>
      </Dialog>

      <Dialog open={copyModalOpen} onClose={() => setCopyModalOpen(false)}>
        <DialogTitle>Select Local Repository</DialogTitle>
        <DialogContent>
          <TextField
            select
            label="Target Repository"
            fullWidth
            margin="dense"
            value={copyTargetRepo}
            onChange={e => setCopyTargetRepo(e.target.value)}
          >
            {repos
              .filter(r => r.type === 'LocalStorage')
              .map(r => (
                <MenuItem key={r.slug} value={r.slug}>
                  {r.name}
                </MenuItem>
              ))}
          </TextField>
        </DialogContent>
        <DialogActions>
          <Button onClick={() => setCopyModalOpen(false)}>Cancel</Button>
          <Button
            onClick={async () => {
              setLoading(true);
              setCopyModalOpen(false);
              try {
                const res = await axios.post('/api/repos/copy', {
                  source: selectedRepo,
                  target: copyTargetRepo,
                  files: [selectedFile],
                });
                const json = res.data;
                if (json.success) {
                  showToast("success",`Copied to ${copyTargetRepo}`);
                } else {
                  showToast("error",json.error || 'Copy failed');
                }
              } catch (e) {
                showToast("error",'Error copying file');
              } finally {
                setLoading(false);
          
              }
            }}
            variant="contained"
            disabled={!copyTargetRepo}
          >
            Copy
          </Button>
        </DialogActions>
      </Dialog>


    </div>
  );
}

export default App;
