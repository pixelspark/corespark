/* This file is part of CoreSpark. CoreSpark is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * CoreSpark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CoreSpark.  If not, see <http://www.gnu.org/licenses/>. */
 
 #ifndef _TJUPDATERENGINE_H
#define _TJUPDATERENGINE_H

namespace tj {
	namespace updater {
		class Update;

		class UpdateActions {
			public:
				UpdateActions();
				~UpdateActions();
				bool Run(const Update& parent);
				void Load(TiXmlElement* actions);

			protected:
				struct UpdateAction {
					enum ActionType {
						ActionNothing = 0,
						ActionRun = 1,
					};

					UpdateAction(TiXmlElement* action);
					UpdateAction(ActionType at, const std::wstring& file = L"");
					bool Run(const Update& parent);

					ActionType _type;
					std::wstring _file;
					std::wstring _parameters;
				};

				std::deque<UpdateAction> _actions;
		};

		class Update {
			public:
				Update();
				~Update();
				void Load(TiXmlElement* el);
				const std::wstring& GetDescription() const;
				void DownloadResources();
				bool IsReadyToInstall() const;
				bool GetDownloadedResource(const std::wstring& rid, std::wstring& path) const;
				bool Install();

			private:
				std::wstring _description;
				std::map<std::wstring, std::wstring> _resourcesNeeded;
				std::map<std::wstring, std::wstring> _resourcesDownloaded;
				UpdateActions _actions;
		};

		class UpdatableComponent {
			friend class Updater;
			friend class UpdaterDialog;

			public:
				UpdatableComponent(const std::wstring& name = L"", const std::wstring& versionSource = L"", const std::wstring& licenseSource = L"");
				~UpdatableComponent();
				const std::wstring& GetName() const;
				const std::wstring& GetVersion() const;
				void Load(TiXmlElement* el);
				bool CanBeUpdated() const;
				bool UpdatesAvailable() const;
				void FindAvailableUpdates();
				void DownloadResourcesForUpdates();
				bool CheckedForUpdates() const;
				void SetCheckedForUpdates(bool c);

			protected:
				void CalculateCurrentVersion();

				std::deque<Update> _updates;

			private:
				std::wstring _name;
				std::wstring _versionSource;
				std::wstring _versionHash;
				std::wstring _licenseHash;
				std::wstring _licenseSource;
				std::wstring _updateURL;
				bool _checked;
		};

		class Updater {
			friend class UpdaterDialog;

			public:
				Updater();
				~Updater();
				void Load(TiXmlElement* root);
				void FindAvailableUpdates();
				void Dump();
				bool UpdatesAvailable() const;
				void DownloadResources();
				bool InstallUpdates();

			private:
				std::deque<UpdatableComponent> _components;
		};
	}
}

#endif